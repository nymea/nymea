/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \page mockdevices.html
    \title Mock devices
    \brief Devices for the nymea test base.

    \ingroup plugins
    \ingroup nymea-tests

    The mock devices are used for testing.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.
*/

#include "devicepluginmock.h"
#include "httpdaemon.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QColor>
#include <QStringList>


DevicePluginMock::DevicePluginMock()
{

}

DevicePluginMock::~DevicePluginMock()
{
}

DeviceManager::DeviceError DevicePluginMock::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if (deviceClassId == mockDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitDevicesDiscovered()));
        return DeviceManager::DeviceErrorAsync;
    } else if (deviceClassId == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock push button discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockPushButtonDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitPushButtonDevicesDiscovered()));
        return DeviceManager::DeviceErrorAsync;
    } else if (deviceClassId == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock display pin discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockDisplayPinDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitDisplayPinDevicesDiscovered()));
        return DeviceManager::DeviceErrorAsync;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

DeviceManager::DeviceSetupStatus DevicePluginMock::setupDevice(Device *device)
{
    if (device->deviceClassId() == mockDeviceClassId || device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        bool async = false;
        bool broken = false;
        if (device->deviceClassId() == mockDeviceClassId) {
            async = device->paramValue(mockDeviceAsyncParamTypeId).toBool();
            broken = device->paramValue(mockDeviceBrokenParamTypeId).toBool();
        } else {
            async = device->paramValue(mockDeviceAutoDeviceAsyncParamTypeId).toBool();
            broken = device->paramValue(mockDeviceAutoDeviceBrokenParamTypeId).toBool();
        }

        if (broken) {
            qCWarning(dcMockDevice) << "This device is intentionally broken.";
            return DeviceManager::DeviceSetupStatusFailure;
        }

        HttpDaemon *daemon = new HttpDaemon(device, this);
        m_daemons.insert(device, daemon);

        if (!daemon->isListening()) {
            qCWarning(dcMockDevice) << "HTTP port opening failed:" << device->paramValue(mockDeviceHttpportParamTypeId).toInt();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
        connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);
        // Keep this queued or it might happen that the HttpDaemon is deleted before it is able to reply to the caller
        connect(daemon, &HttpDaemon::disappear, this, &DevicePluginMock::onDisappear, Qt::QueuedConnection);

        if (async) {
            m_asyncSetupDevices.append(device);
            QTimer::singleShot(1000, this, SLOT(emitDeviceSetupFinished()));
            return DeviceManager::DeviceSetupStatusAsync;
        }
        return DeviceManager::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup PushButton mock device" << device->params();
        return DeviceManager::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup DisplayPin mock device" << device->params();
        return DeviceManager::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockParentDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Parent mock device" << device->params();
        return DeviceManager::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockChildDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Child mock device" << device->params();
        return DeviceManager::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockInputTypeDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup InputType mock device" << device->params();
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginMock::postSetupDevice(Device *device)
{
    qCDebug(dcMockDevice) << "Postsetup mockdevice" << device->name();
    if (device->deviceClassId() == mockParentDeviceClassId) {
        foreach (Device *d, myDevices()) {
            if (d->deviceClassId() == mockChildDeviceClassId && d->parentId() == device->id()) {
                return;
            }
        }
        onChildDeviceDiscovered(device->id());
    }
}

void DevicePluginMock::deviceRemoved(Device *device)
{
    delete m_daemons.take(device);
}

void DevicePluginMock::startMonitoringAutoDevices()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == mockDeviceAutoDeviceClassId) {
            return; // We already have a Auto Mock device... do nothing.
        }
    }

    DeviceDescriptor mockDescriptor(mockDeviceAutoDeviceClassId, "Mock Device (Auto created)");

    ParamList params;
    qsrand(QDateTime::currentMSecsSinceEpoch());
    int port = 4242 + (qrand() % 1000);
    Param param(mockDeviceAutoDeviceHttpportParamTypeId, port);
    params.append(param);
    mockDescriptor.setParams(params);

    QList<DeviceDescriptor> deviceDescriptorList;
    deviceDescriptorList.append(mockDescriptor);

    emit autoDevicesAppeared(mockDeviceAutoDeviceClassId, deviceDescriptorList);
}

DeviceManager::DeviceSetupStatus DevicePluginMock::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret)
{
    Q_UNUSED(params)
    Q_UNUSED(secret)

    qCDebug(dcMockDevice) << "Confirm pairing";

    if (deviceClassId == mockPushButtonDeviceClassId) {
        if (!m_pushbuttonPressed) {
            qCDebug(dcMockDevice) << "PushButton not pressed yet!";
            return DeviceManager::DeviceSetupStatusFailure;
        }

        m_pairingId = pairingTransactionId;
        QTimer::singleShot(1000, this, SLOT(onPushButtonPairingFinished()));
        return DeviceManager::DeviceSetupStatusAsync;
    } else if (deviceClassId == mockDisplayPinDeviceClassId) {
        if (secret != "243681") {
            qCWarning(dcMockDevice) << "Invalid pin:" << secret;
            return DeviceManager::DeviceSetupStatusFailure;
        }
        m_pairingId = pairingTransactionId;
        QTimer::singleShot(500, this, SLOT(onDisplayPinPairingFinished()));
        return DeviceManager::DeviceSetupStatusAsync;
    }

    qCWarning(dcMockDevice) << "Invalid deviceclassId -> no pairing possible with this device";
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginMock::displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor)
{
    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(deviceDescriptor)

    qCDebug(dcMockDevice) << QString(tr("Display pin!! The pin is 243681"));

    return DeviceManager::DeviceErrorNoError;
}

DeviceManager::DeviceError DevicePluginMock::executeAction(Device *device, const Action &action)
{
    if (!myDevices().contains(device))
        return DeviceManager::DeviceErrorDeviceNotFound;

    if (device->deviceClassId() == mockDeviceClassId) {
        if (action.actionTypeId() == mockMockAsyncActionTypeId || action.actionTypeId() == mockMockAsyncFailingActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return DeviceManager::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockMockFailingActionTypeId)
            return DeviceManager::DeviceErrorSetupFailed;

        if (action.actionTypeId() == mockPowerActionTypeId) {
            qCDebug(dcMockDevice()) << "Setting power to" << action.param(mockPowerActionPowerParamTypeId).value().toBool();
            device->setStateValue(mockPowerStateTypeId, action.param(mockPowerActionPowerParamTypeId).value().toBool());
        }

        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return DeviceManager::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        if (action.actionTypeId() == mockDeviceAutoMockActionAsyncActionTypeId || action.actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return DeviceManager::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockDeviceAutoMockActionBrokenActionTypeId)
            return DeviceManager::DeviceErrorSetupFailed;

        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return DeviceManager::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId) {
        if (action.actionTypeId() == mockPushButtonColorActionTypeId) {
            QString colorString = action.param(mockPushButtonColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return DeviceManager::DeviceErrorInvalidParameter;
            }
            device->setStateValue(mockPushButtonColorStateTypeId, colorString);
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonPercentageActionTypeId) {
            device->setStateValue(mockPushButtonPercentageStateTypeId, action.param(mockPushButtonPercentageActionPercentageParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonAllowedValuesActionTypeId) {
            device->setStateValue(mockPushButtonAllowedValuesStateTypeId, action.param(mockPushButtonAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonDoubleActionTypeId) {
            device->setStateValue(mockPushButtonDoubleStateTypeId, action.param(mockPushButtonDoubleActionDoubleParamTypeId).value().toDouble());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonBoolActionTypeId) {
            device->setStateValue(mockPushButtonBoolStateTypeId, action.param(mockPushButtonBoolActionBoolParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonTimeoutActionTypeId) {
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockDisplayPinDeviceClassId) {
        if (action.actionTypeId() == mockDisplayPinColorActionTypeId) {
            QString colorString = action.param(mockDisplayPinColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return DeviceManager::DeviceErrorInvalidParameter;
            }
            device->setStateValue(mockDisplayPinColorStateTypeId, colorString);
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinPercentageActionTypeId) {
            device->setStateValue(mockDisplayPinPercentageStateTypeId, action.param(mockDisplayPinPercentageActionPercentageParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinAllowedValuesActionTypeId) {
            device->setStateValue(mockDisplayPinAllowedValuesStateTypeId, action.param(mockDisplayPinAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinDoubleActionTypeId) {
            device->setStateValue(mockDisplayPinDoubleStateTypeId, action.param(mockDisplayPinDoubleActionDoubleParamTypeId).value().toDouble());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinBoolActionTypeId) {
            device->setStateValue(mockDisplayPinBoolStateTypeId, action.param(mockDisplayPinBoolActionBoolParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinTimeoutActionTypeId) {
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockParentDeviceClassId) {
        if (action.actionTypeId() == mockParentBoolValueActionTypeId) {
            device->setStateValue(mockParentBoolValueStateTypeId, action.param(mockParentBoolValueActionBoolValueParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockChildDeviceClassId) {
        if (action.actionTypeId() == mockChildBoolValueActionTypeId) {
            device->setStateValue(mockChildBoolValueStateTypeId, action.param(mockChildBoolValueActionBoolValueParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockInputTypeDeviceClassId) {
        if (action.actionTypeId() == mockInputTypeWritableBoolActionTypeId) {
            device->setStateValue(mockInputTypeWritableBoolStateTypeId, action.param(mockInputTypeWritableBoolActionWritableBoolParamTypeId).value().toULongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableIntStateTypeId, action.param(mockInputTypeWritableIntActionWritableIntParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableIntMinMaxActionTypeId) {
            device->setStateValue(mockInputTypeWritableIntMinMaxStateTypeId, action.param(mockInputTypeWritableIntMinMaxActionWritableIntMinMaxParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableUIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableUIntStateTypeId, action.param(mockInputTypeWritableUIntActionWritableUIntParamTypeId).value().toULongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableUIntMinMaxActionTypeId) {
            device->setStateValue(mockInputTypeWritableUIntMinMaxStateTypeId, action.param(mockInputTypeWritableUIntMinMaxActionWritableUIntMinMaxParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableDoubleActionTypeId) {
            device->setStateValue(mockInputTypeWritableDoubleStateTypeId, action.param(mockInputTypeWritableDoubleActionWritableDoubleParamTypeId).value().toDouble());
        } else if (action.actionTypeId() == mockInputTypeWritableDoubleMinMaxActionTypeId) {
            device->setStateValue(mockInputTypeWritableDoubleMinMaxStateTypeId, action.param(mockInputTypeWritableDoubleMinMaxActionWritableDoubleMinMaxParamTypeId).value().toDouble());
        } else if (action.actionTypeId() == mockInputTypeWritableStringActionTypeId) {
            device->setStateValue(mockInputTypeWritableStringStateTypeId, action.param(mockInputTypeWritableStringActionWritableStringParamTypeId).value().toString());
        } else if (action.actionTypeId() == mockInputTypeWritableStringSelectionActionTypeId) {
            device->setStateValue(mockInputTypeWritableStringSelectionStateTypeId, action.param(mockInputTypeWritableStringSelectionActionWritableStringSelectionParamTypeId).value().toString());
        } else if (action.actionTypeId() == mockInputTypeWritableColorActionTypeId) {
            device->setStateValue(mockInputTypeWritableColorStateTypeId, action.param(mockInputTypeWritableColorActionWritableColorParamTypeId).value().toString());
        } else if (action.actionTypeId() == mockInputTypeWritableTimeActionTypeId) {
            device->setStateValue(mockInputTypeWritableTimeStateTypeId, action.param(mockInputTypeWritableTimeActionWritableTimeParamTypeId).value().toTime());
        } else if (action.actionTypeId() == mockInputTypeWritableTimestampIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableTimestampIntStateTypeId, action.param(mockInputTypeWritableTimestampIntActionWritableTimestampIntParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableTimestampUIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableTimestampUIntStateTypeId, action.param(mockInputTypeWritableTimestampUIntActionWritableTimestampUIntParamTypeId).value().toULongLong());
        }

    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginMock::setState(const StateTypeId &stateTypeId, const QVariant &value)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Device *device = m_daemons.key(daemon);
    device->setStateValue(stateTypeId, value);
}

void DevicePluginMock::triggerEvent(const EventTypeId &id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Device *device = m_daemons.key(daemon);

    Event event(id, device->id());

    qCDebug(dcMockDevice) << "Emitting event " << event.eventTypeId();
    emit emitEvent(event);
}

void DevicePluginMock::onDisappear()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }
    Device *device = m_daemons.key(daemon);
    qCDebug(dcMockDevice) << "Emitting autoDeviceDisappeared for device" << device->id();
    emit autoDeviceDisappeared(device->id());
}

void DevicePluginMock::emitDevicesDiscovered()
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockDeviceClassId, "Mock Device 1 (Discovered)", "55555");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55555");
        params.append(httpParam);
        d1.setParams(params);
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDeviceClassId, "Mock Device 2 (Discovered)", "55556");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55556");
        params.append(httpParam);
        d2.setParams(params);
        deviceDescriptors.append(d2);
    }

    emit devicesDiscovered(mockDeviceClassId, deviceDescriptors);
}

void DevicePluginMock::emitPushButtonDevicesDiscovered()
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockPushButtonDeviceClassId, "Mock Device (Push Button)", "1");
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockPushButtonDeviceClassId, "Mock Device (Push Button)", "2");
        deviceDescriptors.append(d2);
    }
    emit devicesDiscovered(mockPushButtonDeviceClassId, deviceDescriptors);

    m_pushbuttonPressed = false;
    QTimer::singleShot(3000, this, SLOT(onPushButtonPressed()));
    qCDebug(dcMockDevice) << "Start PushButton timer (will be pressed in 3 second)";
}

void DevicePluginMock::emitDisplayPinDevicesDiscovered()
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockDisplayPinDeviceClassId, "Mock Device (Display Pin)", "1");
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDisplayPinDeviceClassId, "Mock Device (Display Pin)", "2");
        deviceDescriptors.append(d2);
    }

    emit devicesDiscovered(mockDisplayPinDeviceClassId, deviceDescriptors);
}

void DevicePluginMock::onPushButtonPressed()
{
    qCDebug(dcMockDevice) << "PushButton pressed (automatically)";
    m_pushbuttonPressed = true;
}

void DevicePluginMock::emitDeviceSetupFinished()
{
    qCDebug(dcMockDevice) << "Emitting setup finised";
    Device *device = m_asyncSetupDevices.takeFirst();
    if (device->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
    } else {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
    }
}

void DevicePluginMock::emitActionExecuted()
{
    QPair<Action, Device*> action = m_asyncActions.takeFirst();
    if (action.first.actionTypeId() == mockMockAsyncActionTypeId) {
        m_daemons.value(action.second)->actionExecuted(action.first.actionTypeId());
        emit actionExecutionFinished(action.first.id(), DeviceManager::DeviceErrorNoError);
    } else if (action.first.actionTypeId() == mockMockAsyncFailingActionTypeId) {
        emit actionExecutionFinished(action.first.id(), DeviceManager::DeviceErrorSetupFailed);
    }
}

void DevicePluginMock::onPushButtonPairingFinished()
{
    qCDebug(dcMockDevice) << "Pairing PushButton Device finished";
    emit pairingFinished(m_pairingId, DeviceManager::DeviceSetupStatusSuccess);
}

void DevicePluginMock::onDisplayPinPairingFinished()
{
    qCDebug(dcMockDevice) << "Pairing DisplayPin Device finished";
    emit pairingFinished(m_pairingId, DeviceManager::DeviceSetupStatusSuccess);
}

void DevicePluginMock::onChildDeviceDiscovered(const DeviceId &parentId)
{
    qCDebug(dcMockDevice) << "Child device discovered for parent" << parentId.toString();
    DeviceDescriptor mockDescriptor(mockChildDeviceClassId, "Child Mock Device (Auto created)", "Child Mock Device (Auto created)", parentId);
    emit autoDevicesAppeared(mockChildDeviceClassId, QList<DeviceDescriptor>() << mockDescriptor);
}

void DevicePluginMock::onPluginConfigChanged()
{

}
