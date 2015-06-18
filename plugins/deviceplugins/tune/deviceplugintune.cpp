/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QDebug>
#include <QStringList>
#include <QJsonDocument>

#include "deviceplugintune.h"
#include "devicemanager.h"
#include "plugininfo.h"

DevicePluginTune::DevicePluginTune()
{
    m_server = new JsonRpcServer(this);

    connect(m_server, &JsonRpcServer::connectionStatusChanged, this, &DevicePluginTune::tuneConnectionStatusChanged);
    connect(m_server, &JsonRpcServer::gotMoodSync, this, &DevicePluginTune::updateMood);
    connect(m_server, &JsonRpcServer::gotTuneSync, this, &DevicePluginTune::updateTune);
    connect(m_server, &JsonRpcServer::gotActionResponse, this, &DevicePluginTune::processActionResponse);
}

DeviceManager::HardwareResources DevicePluginTune::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginTune::startMonitoringAutoDevices()
{
    m_server->start();
}

DeviceManager::DeviceSetupStatus DevicePluginTune::setupDevice(Device *device)
{
    // tune
    if (device->deviceClassId() == tuneDeviceClassId && !tuneAlreadyAdded()) {
        m_tuneDeviceId = device->id();
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    // mood
    if ((device->deviceClassId() == moodDeviceClassId) && tuneAlreadyAdded()) {

        // check index position
        int position = device->paramValue("position").toInt();

        if (position >= myDevices().count()) {
            device->setParamValue("position", myDevices().count());
        } else {
            if (position <= 0) {
                device->setParamValue("position", 0);
                position = 0;
            }
            foreach (Device *d, myDevices()) {
                int currentPosition = d->paramValue("position").toInt();
                if (currentPosition >= position) {
                    d->setParamValue("position", currentPosition + 1);
                }
            }
        }

        device->setName(device->paramValue("name").toString() + " (Mood)");
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginTune::postSetupDevice(Device *device)
{
    Q_UNUSED(device)
    sync();
}

void DevicePluginTune::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == moodDeviceClassId) {
        int position = device->paramValue("position").toInt();

        foreach (Device *d, myDevices()) {
            int currentPosition = d->paramValue("position").toInt();
            if (currentPosition >= position ) {
                d->setParamValue("position", currentPosition - 1);
            }
        }
        sync();
    }
    if (device->deviceClassId() == tuneDeviceClassId && m_server->tuneAvailable()) {
        tuneAutodetected();
    }
}

bool DevicePluginTune::sync()
{
    return m_server->sync(myDevices());
}

bool DevicePluginTune::tuneAlreadyAdded()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == tuneDeviceClassId) {
            return true;
        }
    }
    return false;
}

void DevicePluginTune::tuneAutodetected()
{
    QList<DeviceDescriptor> descriptorList;
    DeviceDescriptor descriptor(tuneDeviceClassId);
    ParamList params;
    params.append(Param("name", "Living room"));
    descriptor.setParams(params);
    descriptorList.append(descriptor);
    metaObject()->invokeMethod(this, "autoDevicesAppeared", Qt::QueuedConnection, Q_ARG(DeviceClassId, tuneDeviceClassId), Q_ARG(QList<DeviceDescriptor>, descriptorList));
}

void DevicePluginTune::tuneConnectionStatusChanged(const bool &connected)
{
    if (connected) {
        if (!tuneAlreadyAdded()) {
            tuneAutodetected();
        } else {
            Device *device = deviceManager()->findConfiguredDevice(m_tuneDeviceId);
            device->setStateValue(reachableStateTypeId, true);
        }
        sync();
    } else {
        Device *device = deviceManager()->findConfiguredDevice(m_tuneDeviceId);
        device->setStateValue(reachableStateTypeId, false);
    }
}

void DevicePluginTune::updateMood(const QVariantMap &message)
{
    QVariantMap mood = message.value("mood").toMap();
    //qDebug () << QJsonDocument::fromVariant(message).toJson();
    Device *device = deviceManager()->findConfiguredDevice(DeviceId(mood.value("deviceId").toString()));
    if (device) {
        QVariantMap states = mood.value("states").toMap();
        device->setStateValue(activeStateTypeId, states.value("active").toBool());
        device->setStateValue(valueStateTypeId, states.value("value").toInt());
    }
}

void DevicePluginTune::updateTune(const QVariantMap &message)
{
    QVariantMap tune = message.value("tune").toMap();
    Device *device = deviceManager()->findConfiguredDevice(DeviceId(tune.value("deviceId").toString()));
    if (device) {
        QVariantMap states = tune.value("states").toMap();
        device->setStateValue(powerStateTypeId, states.value("power").toBool());
        device->setStateValue(brightnessStateTypeId, states.value("brigthness").toInt());
        device->setStateValue(approximationDetectedStateTypeId, states.value("approximationDetected").toBool());
        device->setStateValue(lightIntensityStateTypeId, states.value("lightIntensity").toInt());
        device->setStateValue(humidityStateTypeId, states.value("humidity").toInt());
        device->setStateValue(temperatureStateTypeId, states.value("temperature").toDouble());
    }
}

void DevicePluginTune::processActionResponse(const QVariantMap &message)
{
    bool success = message.value("success").toBool();
    ActionId actionId = ActionId(message.value("actionId").toString());
    if (success) {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
    } else {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
    }
}

DeviceManager::DeviceError DevicePluginTune::executeAction(Device *device, const Action &action)
{
    if (!m_server->tuneAvailable()) {
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // check DeviceClassId
    if (device->deviceClassId() != moodDeviceClassId && device->deviceClassId() != tuneDeviceClassId) {
        return DeviceManager::DeviceErrorDeviceClassNotFound;
    }

    // check ActionTypeId
    if (action.actionTypeId() != powerActionTypeId &&
            action.actionTypeId() != brightnessActionTypeId &&
            action.actionTypeId() != valueActionTypeId &&
            action.actionTypeId() != activeActionTypeId){
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // request the action execution on tune
    m_server->executeAction(device, action);
    return DeviceManager::DeviceErrorAsync;
}

