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

#include "devices/device.h"
#include "plugininfo.h"

#include <QDebug>
#include <QColor>
#include <QStringList>
#include <QTimer>

DevicePluginMock::DevicePluginMock()
{

}

DevicePluginMock::~DevicePluginMock()
{
}

Device::DeviceError DevicePluginMock::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if (deviceClassId == mockDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitDevicesDiscovered()));
        return Device::DeviceErrorAsync;
    } else if (deviceClassId == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock push button discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockPushButtonDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitPushButtonDevicesDiscovered()));
        return Device::DeviceErrorAsync;
    } else if (deviceClassId == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock display pin discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockDisplayPinDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, SLOT(emitDisplayPinDevicesDiscovered()));
        return Device::DeviceErrorAsync;
    } else if (deviceClassId == mockParentDeviceClassId) {
        qCDebug(dcMockDevice()) << "Starting discovery for mock device parent";
        QTimer::singleShot(1000, this, [this](){
            DeviceDescriptor descriptor(mockParentDeviceClassId, "Mock Parent (Discovered)");
            emit devicesDiscovered(mockParentDeviceClassId, {descriptor});

        });
        return Device::DeviceErrorAsync;
    } else if (deviceClassId == mockChildDeviceClassId) {
        QTimer::singleShot(1000, this, [this](){
            QList<DeviceDescriptor> descriptors;
            if (!myDevices().filterByDeviceClassId(mockParentDeviceClassId).isEmpty()) {
                Device *parent = myDevices().filterByDeviceClassId(mockParentDeviceClassId).first();
                DeviceDescriptor descriptor(mockChildDeviceClassId, "Mock Child (Discovered)", QString(), parent->id());
                descriptors.append(descriptor);
            }
            emit devicesDiscovered(mockChildDeviceClassId, descriptors);
        });
        return Device::DeviceErrorAsync;
    }

    qCWarning(dcMockDevice()) << "Cannot discover for deviceClassId" << deviceClassId;
    return Device::DeviceErrorDeviceClassNotFound;
}

Device::DeviceSetupStatus DevicePluginMock::setupDevice(Device *device)
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
            return Device::DeviceSetupStatusFailure;
        }

        HttpDaemon *daemon = new HttpDaemon(device, this);
        m_daemons.insert(device, daemon);

        if (!daemon->isListening()) {
            qCWarning(dcMockDevice) << "HTTP port opening failed:" << device->paramValue(mockDeviceHttpportParamTypeId).toInt();
            return Device::DeviceSetupStatusFailure;
        }

        connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
        connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);
        // Keep this queued or it might happen that the HttpDaemon is deleted before it is able to reply to the caller
        connect(daemon, &HttpDaemon::disappear, this, &DevicePluginMock::onDisappear, Qt::QueuedConnection);
        connect(daemon, &HttpDaemon::reconfigureAutodevice, this, &DevicePluginMock::onReconfigureAutoDevice, Qt::QueuedConnection);

        if (async) {
            m_asyncSetupDevices.append(device);
            QTimer::singleShot(1000, this, SLOT(emitDeviceSetupFinished()));
            return Device::DeviceSetupStatusAsync;
        }
        return Device::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup PushButton mock device" << device->params();
        return Device::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup DisplayPin mock device" << device->params();
        return Device::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockParentDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Parent mock device" << device->params();
        return Device::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockChildDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Child mock device" << device->params();
        return Device::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == mockInputTypeDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup InputType mock device" << device->params();
        return Device::DeviceSetupStatusSuccess;
    }

    return Device::DeviceSetupStatusFailure;
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

Device::DeviceSetupStatus DevicePluginMock::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret)
{
    Q_UNUSED(params)
    Q_UNUSED(secret)

    qCDebug(dcMockDevice) << "Confirm pairing";

    if (deviceClassId == mockPushButtonDeviceClassId) {
        if (!m_pushbuttonPressed) {
            qCDebug(dcMockDevice) << "PushButton not pressed yet!";
            return Device::DeviceSetupStatusFailure;
        }

        m_pairingId = pairingTransactionId;
        QTimer::singleShot(1000, this, SLOT(onPushButtonPairingFinished()));
        return Device::DeviceSetupStatusAsync;
    } else if (deviceClassId == mockDisplayPinDeviceClassId) {
        if (secret != "243681") {
            qCWarning(dcMockDevice) << "Invalid pin:" << secret;
            return Device::DeviceSetupStatusFailure;
        }
        m_pairingId = pairingTransactionId;
        QTimer::singleShot(500, this, SLOT(onDisplayPinPairingFinished()));
        return Device::DeviceSetupStatusAsync;
    }

    qCWarning(dcMockDevice) << "Invalid deviceclassId -> no pairing possible with this device";
    return Device::DeviceSetupStatusFailure;
}

Device::DeviceError DevicePluginMock::displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor)
{
    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(deviceDescriptor)

    qCDebug(dcMockDevice) << QString(tr("Display pin!! The pin is 243681"));

    return Device::DeviceErrorNoError;
}

Device::BrowseResult DevicePluginMock::browseDevice(Device *device, Device::BrowseResult result, const QString &nodeId)
{
    qCDebug(dcMockDevice()) << "Browse device called" << device;
    if (device->deviceClassId() == mockDeviceClassId) {
        if (device->paramValue(mockDeviceAsyncParamTypeId).toBool()) {
            result.status = Device::DeviceErrorAsync;
            QTimer::singleShot(1000, device, [this, device, result, nodeId]() mutable {
                if (device->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
                    result.status = Device::DeviceErrorHardwareFailure;
                } else {
                    result = generateBrowseItems(nodeId, result);
                }
                emit browseRequestFinished(result);
            });
        }
        else if (device->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
            result.status = Device::DeviceErrorHardwareFailure;
        } else {
            result = generateBrowseItems(nodeId, result);
        }
    }
    return result;
}

Device::DeviceError DevicePluginMock::executeAction(Device *device, const Action &action)
{
    if (!myDevices().contains(device))
        return Device::DeviceErrorDeviceNotFound;

    if (device->deviceClassId() == mockDeviceClassId) {
        if (action.actionTypeId() == mockAsyncActionTypeId || action.actionTypeId() == mockAsyncFailingActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return Device::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockFailingActionTypeId)
            return Device::DeviceErrorSetupFailed;

        if (action.actionTypeId() == mockPowerActionTypeId) {
            qCDebug(dcMockDevice()) << "Setting power to" << action.param(mockPowerActionPowerParamTypeId).value().toBool();
            device->setStateValue(mockPowerStateTypeId, action.param(mockPowerActionPowerParamTypeId).value().toBool());
        }
        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return Device::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        if (action.actionTypeId() == mockDeviceAutoMockActionAsyncActionTypeId || action.actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return Device::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockDeviceAutoMockActionBrokenActionTypeId)
            return Device::DeviceErrorSetupFailed;

        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return Device::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId) {
        if (action.actionTypeId() == mockPushButtonColorActionTypeId) {
            QString colorString = action.param(mockPushButtonColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return Device::DeviceErrorInvalidParameter;
            }
            device->setStateValue(mockPushButtonColorStateTypeId, colorString);
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonPercentageActionTypeId) {
            device->setStateValue(mockPushButtonPercentageStateTypeId, action.param(mockPushButtonPercentageActionPercentageParamTypeId).value().toInt());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonAllowedValuesActionTypeId) {
            device->setStateValue(mockPushButtonAllowedValuesStateTypeId, action.param(mockPushButtonAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonDoubleActionTypeId) {
            device->setStateValue(mockPushButtonDoubleStateTypeId, action.param(mockPushButtonDoubleActionDoubleParamTypeId).value().toDouble());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonBoolActionTypeId) {
            device->setStateValue(mockPushButtonBoolStateTypeId, action.param(mockPushButtonBoolActionBoolParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonTimeoutActionTypeId) {
            return Device::DeviceErrorAsync;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockDisplayPinDeviceClassId) {
        if (action.actionTypeId() == mockDisplayPinColorActionTypeId) {
            QString colorString = action.param(mockDisplayPinColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return Device::DeviceErrorInvalidParameter;
            }
            device->setStateValue(mockDisplayPinColorStateTypeId, colorString);
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinPercentageActionTypeId) {
            device->setStateValue(mockDisplayPinPercentageStateTypeId, action.param(mockDisplayPinPercentageActionPercentageParamTypeId).value().toInt());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinAllowedValuesActionTypeId) {
            device->setStateValue(mockDisplayPinAllowedValuesStateTypeId, action.param(mockDisplayPinAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinDoubleActionTypeId) {
            device->setStateValue(mockDisplayPinDoubleStateTypeId, action.param(mockDisplayPinDoubleActionDoubleParamTypeId).value().toDouble());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinBoolActionTypeId) {
            device->setStateValue(mockDisplayPinBoolStateTypeId, action.param(mockDisplayPinBoolActionBoolParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinTimeoutActionTypeId) {
            return Device::DeviceErrorAsync;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockParentDeviceClassId) {
        if (action.actionTypeId() == mockParentBoolValueActionTypeId) {
            device->setStateValue(mockParentBoolValueStateTypeId, action.param(mockParentBoolValueActionBoolValueParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockChildDeviceClassId) {
        if (action.actionTypeId() == mockChildBoolValueActionTypeId) {
            device->setStateValue(mockChildBoolValueStateTypeId, action.param(mockChildBoolValueActionBoolValueParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        }
        return Device::DeviceErrorActionTypeNotFound;
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
    return Device::DeviceErrorDeviceClassNotFound;
}

Device::DeviceError DevicePluginMock::executeBrowserItem(Device *device, const BrowserItemAction &browserItemAction)
{
    bool broken = device->paramValue(mockDeviceBrokenParamTypeId).toBool();
    bool async = device->paramValue(mockDeviceAsyncParamTypeId).toBool();

    if (!async){
        if (broken) {
            return Device::DeviceErrorHardwareFailure;
        }
        return Device::DeviceErrorNoError;
    }

    QTimer::singleShot(2000, device, [this, broken, browserItemAction](){
        emit this->browserItemExecutionFinished(browserItemAction.id(), broken ? Device::DeviceErrorHardwareFailure : Device::DeviceErrorNoError);
    });
    return Device::DeviceErrorAsync;
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

void DevicePluginMock::onReconfigureAutoDevice()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon *>(sender());
    if (!daemon)
        return;

    Device *device = m_daemons.key(daemon);
    qCDebug(dcMockDevice()) << "Reconfigure auto device for" << device << device->params();

    int currentPort = device->params().paramValue(mockDeviceAutoDeviceHttpportParamTypeId).toInt();

    // Note: the reconfigure makes the http server listen on port + 1
    ParamList params;
    params.append(Param(mockDeviceAutoDeviceHttpportParamTypeId, currentPort + 1));

    DeviceDescriptor deviceDescriptor;
    deviceDescriptor.setTitle(device->name() + " (reconfigured)");
    deviceDescriptor.setDescription("This auto device was reconfigured");
    deviceDescriptor.setDeviceId(device->id());
    deviceDescriptor.setParams(params);

    emit autoDevicesAppeared(mockDeviceAutoDeviceClassId, { deviceDescriptor });
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
        foreach (Device *d, myDevices()) {
            if (d->deviceClassId() == mockDeviceClassId && d->paramValue(mockDeviceHttpportParamTypeId).toInt() == 55555) {
                d1.setDeviceId(d->id());
                break;
            }
        }
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDeviceClassId, "Mock Device 2 (Discovered)", "55556");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55556");
        params.append(httpParam);
        d2.setParams(params);
        foreach (Device *d, myDevices()) {
            if (d->deviceClassId() == mockDeviceClassId && d->paramValue(mockDeviceHttpportParamTypeId).toInt() == 55556) {
                d2.setDeviceId(d->id());
                break;
            }
        }
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
        foreach (Device *existingDev, myDevices()) {
            if (existingDev->deviceClassId() == mockDisplayPinDeviceClassId) {
                d1.setDeviceId(existingDev->id());
                break;
            }
        }
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDisplayPinDeviceClassId, "Mock Device (Display Pin)", "2");
        int count = 0;
        foreach (Device *existingDev, myDevices()) {
            if (existingDev->deviceClassId() == mockDisplayPinDeviceClassId && ++count > 1) {
                d2.setDeviceId(existingDev->id());
                break;
            }
        }
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
        emit deviceSetupFinished(device, Device::DeviceSetupStatusFailure);
    } else {
        emit deviceSetupFinished(device, Device::DeviceSetupStatusSuccess);
    }
}

void DevicePluginMock::emitActionExecuted()
{
    QPair<Action, Device*> action = m_asyncActions.takeFirst();
    if (action.first.actionTypeId() == mockAsyncActionTypeId) {
        m_daemons.value(action.second)->actionExecuted(action.first.actionTypeId());
        emit actionExecutionFinished(action.first.id(), Device::DeviceErrorNoError);
    } else if (action.first.actionTypeId() == mockAsyncFailingActionTypeId) {
        emit actionExecutionFinished(action.first.id(), Device::DeviceErrorSetupFailed);
    }
}

void DevicePluginMock::onPushButtonPairingFinished()
{
    qCDebug(dcMockDevice) << "Pairing PushButton Device finished";
    emit pairingFinished(m_pairingId, Device::DeviceSetupStatusSuccess);
}

void DevicePluginMock::onDisplayPinPairingFinished()
{
    qCDebug(dcMockDevice) << "Pairing DisplayPin Device finished";
    emit pairingFinished(m_pairingId, Device::DeviceSetupStatusSuccess);
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

Device::BrowseResult DevicePluginMock::generateBrowseItems(const QString &itemId, Device::BrowseResult result)
{
    result.status = Device::DeviceErrorNoError;

    if (itemId.isEmpty()) {
        BrowserItem item = BrowserItem("0", "Item 0", true);
        item.setDescription("I'm a folder");
        item.setIcon(BrowserItem::BrowserIconFolder);
        result.items.append(item);

        item = BrowserItem("1", "Item 1", false, true);
        item.setDescription("I'm executable");
        item.setIcon(BrowserItem::BrowserIconApplication);
        result.items.append(item);

        item = BrowserItem("2", "Item 2", false, true);
        item.setDescription("I'm a file");
        item.setIcon(BrowserItem::BrowserIconFile);
        result.items.append(item);

        item = BrowserItem("3", "Item 3", false, true);
        item.setDescription("I have a nice thumbnail");
        item.setIcon(BrowserItem::BrowserIconFile);
        item.setThumbnail("https://github.com/guh/nymea/raw/master/icons/nymea-logo-256x256.png");
        result.items.append(item);

        item = BrowserItem("4", "Item 4", false, false);
        item.setDescription("I'm disabled");
        item.setIcon(BrowserItem::BrowserIconFile);
        result.items.append(item);

    }
    else if (itemId == "0") {
        result.items.append(BrowserItem("5", "Item 5"));
        result.items.append(BrowserItem("6", "Item 6"));
        result.items.append(BrowserItem("7", "Item 7"));
        result.items.append(BrowserItem("8", "Item 8"));
        result.items.append(BrowserItem("9", "Item 9"));
    }
    else if (itemId == "2") {
        result.items.append(BrowserItem("10", "Item 10", true));
        result.items.append(BrowserItem("11", "Item 11"));
        result.items.append(BrowserItem("12", "Item 12"));
        result.items.append(BrowserItem("13", "Item 13"));
        result.items.append(BrowserItem("14", "Item 14"));
    }
    else if (itemId == "10") {
        result.items.append(BrowserItem("15", "Item 15"));
        result.items.append(BrowserItem("16", "Item 16"));
    } else {
        result.status = Device::DeviceErrorInvalidParameter;
    }

    return result;
}
