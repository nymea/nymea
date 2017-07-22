/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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
    \brief Devices for the guh test base.

    \ingroup plugins
    \ingroup guh-tests

    The mock devices are used for testing.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/mock/devicepluginmock.json
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

DeviceManager::HardwareResources DevicePluginMock::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceError DevicePluginMock::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if (deviceClassId == mockDeviceClassId || deviceClassId == mockDeviceAutoDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(resultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitDevicesDiscovered()));
        return DeviceManager::DeviceErrorAsync;
    } else if (deviceClassId == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock push button discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(resultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitPushButtonDevicesDiscovered()));
        return DeviceManager::DeviceErrorAsync;
    } else if (deviceClassId == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock display pin discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(resultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitDisplayPinDevicesDiscovered()));
        return DeviceManager::DeviceErrorAsync;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

DeviceManager::DeviceSetupStatus DevicePluginMock::setupDevice(Device *device)
{
    if (device->deviceClassId() == mockDeviceClassId || device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        qCDebug(dcMockDevice) << "Mockdevice created returning true"
                              << device->paramValue(httpportParamTypeId).toInt()
                              << device->paramValue(asyncParamTypeId).toBool()
                              << device->paramValue(brokenParamTypeId).toBool();

        if (device->paramValue(brokenParamTypeId).toBool()) {
            qCWarning(dcMockDevice) << "This device is intentionally broken.";
            return DeviceManager::DeviceSetupStatusFailure;
        }

        HttpDaemon *daemon = new HttpDaemon(device, this);
        m_daemons.insert(device, daemon);

        if (!daemon->isListening()) {
            qCWarning(dcMockDevice) << "HTTP port opening failed.";
            return DeviceManager::DeviceSetupStatusFailure;
        }

        connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
        connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);

        if (device->paramValue(asyncParamTypeId).toBool()) {
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
        device->setParentId(DeviceId(device->params().paramValue(parentUuidParamTypeId).toString()));
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
    Param param(httpportParamTypeId, port);
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
    if (device->deviceClassId() == mockDeviceClassId || device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        if (!myDevices().contains(device))
            return DeviceManager::DeviceErrorDeviceNotFound;

        if (action.actionTypeId() == mockAsyncActionTypeId || action.actionTypeId() == mockAsyncFailingActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return DeviceManager::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockFailingActionTypeId)
            return DeviceManager::DeviceErrorSetupFailed;

        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return DeviceManager::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId || device->deviceClassId() == mockDisplayPinDeviceClassId) {
        if (action.actionTypeId() == colorActionTypeId) {
            QString colorString = action.param(colorStateParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return DeviceManager::DeviceErrorInvalidParameter;
            }
            device->setStateValue(colorStateTypeId, colorString);
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == percentageActionTypeId) {
            device->setStateValue(percentageStateTypeId, action.param(percentageStateParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == allowedValuesActionTypeId) {
            device->setStateValue(allowedValuesStateTypeId, action.param(allowedValuesStateParamTypeId).value().toString());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == doubleActionTypeId) {
            device->setStateValue(doubleStateTypeId, action.param(doubleStateParamTypeId).value().toDouble());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == boolActionTypeId) {
            device->setStateValue(boolValueStateTypeId, action.param(boolValueStateParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == timeoutActionTypeId) {
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockParentDeviceClassId || device->deviceClassId() == mockChildDeviceClassId) {
        if (action.actionTypeId() == boolValueParentActionTypeId) {
            device->setStateValue(boolValueParentStateTypeId, action.param(boolValueParentStateParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
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

void DevicePluginMock::emitDevicesDiscovered()
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockDeviceClassId, "Mock Device 1 (Discovered)", "55555");
        ParamList params;
        Param httpParam(httpportParamTypeId, "55555");
        params.append(httpParam);
        d1.setParams(params);
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDeviceClassId, "Mock Device 2 (Discovered)", "55556");
        ParamList params;
        Param httpParam(httpportParamTypeId, "55556");
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
    if (device->paramValue(brokenParamTypeId).toBool()) {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
    } else {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
    }
}

void DevicePluginMock::emitActionExecuted()
{
    QPair<Action, Device*> action = m_asyncActions.takeFirst();
    if (action.first.actionTypeId() == mockAsyncActionTypeId) {
        m_daemons.value(action.second)->actionExecuted(action.first.actionTypeId());
        emit actionExecutionFinished(action.first.id(), DeviceManager::DeviceErrorNoError);
    } else if (action.first.actionTypeId() == mockAsyncFailingActionTypeId) {
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
    DeviceDescriptor mockDescriptor(mockChildDeviceClassId, "Child Mock Device (Auto created)");
    ParamList params;
    params.append(Param(parentUuidParamTypeId, parentId));
    mockDescriptor.setParams(params);

    emit autoDevicesAppeared(mockChildDeviceClassId, QList<DeviceDescriptor>() << mockDescriptor);
}

void DevicePluginMock::onPluginConfigChanged()
{

}
