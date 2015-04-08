/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
    m_manager = new TuneManager(31337, this);

    connect(m_manager, &TuneManager::tuneConnectionStatusChanged, this, &DevicePluginTune::tuneConnectionStatusChanged);
    connect(m_manager, &TuneManager::dataReady, this, &DevicePluginTune::tuneDataAvailable);
}

DeviceManager::HardwareResources DevicePluginTune::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginTune::startMonitoringAutoDevices()
{
    m_manager->start();
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
    if (device->deviceClassId() == tuneDeviceClassId && m_manager->tuneAvailable()) {
        tuneAutodetected();
    }
}

bool DevicePluginTune::sync()
{
    // sync with devices with tune
    if (!m_manager->tuneAvailable()) {
        return false;
    }

    QVariantMap message;
    QVariantList moods;
    QVariantMap tune;
    foreach (Device* device, myDevices()) {
        if (device->deviceClassId() == moodDeviceClassId) {
            QVariantMap mood;
            mood.insert("name", device->paramValue("name"));
            mood.insert("deviceId", device->id());
            mood.insert("position", device->paramValue("position"));
            mood.insert("icon", device->paramValue("icon"));
            QVariantMap states;
            states.insert("value", device->stateValue(valueStateTypeId).toInt());
            states.insert("active", device->stateValue(activeStateTypeId).toBool());
            mood.insert("states", states);
            moods.append(mood);
        }
        if (device->deviceClassId() == tuneDeviceClassId) {
            tune.insert("name", device->paramValue("name"));
            tune.insert("deviceId", device->id());
            QVariantMap states;
            states.insert("value", device->stateValue(brightnessStateTypeId).toInt());
            states.insert("active", device->stateValue(powerStateTypeId).toBool());
            tune.insert("states", states);
        }
    }

    message.insert("method", "Items.Sync");
    message.insert("moods", moods);
    message.insert("tune", tune);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    qDebug() << jsonDoc.toJson();

    m_manager->sendData(data);
    return true;
}

void DevicePluginTune::syncStates(Device *device)
{
    QVariantMap message;

    if (device->deviceClassId() == moodDeviceClassId) {
        QVariantMap mood;
        QVariantMap states;
        states.insert("value", device->stateValue(valueStateTypeId).toInt());
        states.insert("active", device->stateValue(activeStateTypeId).toBool());
        mood.insert("states", states);
        mood.insert("deviceId", device->id());
        message.insert("method", "Items.SyncStates");
        message.insert("mood", mood);
    }

    if (device->deviceClassId() == tuneDeviceClassId) {
        QVariantMap tune;
        QVariantMap states;
        states.insert("value", device->stateValue(brightnessStateTypeId).toInt());
        states.insert("active", device->stateValue(powerStateTypeId).toBool());
        tune.insert("states", states);
        tune.insert("deviceId", device->id());
        message.insert("method", "Items.SyncStates");
        message.insert("tune", tune);
    }

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    m_manager->sendData(data);

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
    params.append(Param("name", "Wohnzimmer"));
    descriptor.setParams(params);
    descriptorList.append(descriptor);
    metaObject()->invokeMethod(this, "autoDevicesAppeared", Qt::QueuedConnection, Q_ARG(DeviceClassId, tuneDeviceClassId), Q_ARG(QList<DeviceDescriptor>, descriptorList));
}

void DevicePluginTune::activateMood(Device *device)
{
    // first deactivate every current active mood
    foreach (Device* d, myDevices()) {
        if (d->deviceClassId() == moodDeviceClassId) {
            if (d->stateValue(activeStateTypeId).toBool()) {
                d->setStateValue(activeStateTypeId, false);
                syncStates(d);
            }
        }
    }
    device->setStateValue(activeStateTypeId, true);
    syncStates(device);
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

void DevicePluginTune::tuneDataAvailable(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << data << ":" << error.errorString();
    }

    qDebug() << jsonDoc.toJson();

    QVariantMap message = jsonDoc.toVariant().toMap();
    if (message.value("method").toString() == "Items.SyncStates") {
        if (message.contains("mood")) {
            QVariantMap mood = message.value("mood").toMap();
            Device *device = deviceManager()->findConfiguredDevice(DeviceId(mood.value("deviceId").toString()));
            if (device) {
                qDebug() << "update device" << device->name();
                QVariantMap states = mood.value("states").toMap();
                bool activeValue = states.value("active").toBool();
                int value = states.value("value").toInt();
                if (activeValue) {
                    activateMood(device);
                } else {
                    device->setStateValue(activeStateTypeId, activeValue);
                }
                device->setStateValue(valueStateTypeId, value);
            }
        }
        if (message.contains("tune")) {
            QVariantMap tune = message.value("tune").toMap();
            Device *device = deviceManager()->findConfiguredDevice(DeviceId(tune.value("deviceId").toString()));
            if (device) {
                QVariantMap states = tune.value("states").toMap();
                device->setStateValue(powerStateTypeId, states.value("active").toBool());
                device->setStateValue(brightnessStateTypeId, states.value("value").toInt());
                device->setStateValue(approximationDetectedStateTypeId, states.value("approximationDetected").toBool());
                device->setStateValue(lightIntensityStateTypeId, states.value("lightIntensity").toInt());
                device->setStateValue(humidityStateTypeId, states.value("humidity").toInt());
                device->setStateValue(temperatureStateTypeId, states.value("temperature").toDouble());
            }
        }
    }
}

DeviceManager::DeviceError DevicePluginTune::executeAction(Device *device, const Action &action)
{
    if (!m_manager->tuneAvailable()) {
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // Mood
    if (device->deviceClassId() == moodDeviceClassId) {
        if (action.actionTypeId() == activeActionTypeId) {
            bool currentState = device->stateValue(activeStateTypeId).toBool();
            device->setStateValue(activeStateTypeId, !currentState);
            syncStates(device);
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == valueActionTypeId) {
            device->setStateValue(valueStateTypeId, action.param("percentage").value().toInt());
            syncStates(device);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    if (device->deviceClassId() == tuneDeviceClassId) {
        if (action.actionTypeId() == powerActionTypeId) {
            bool currentState = device->stateValue(powerStateTypeId).toBool();
            device->setStateValue(powerStateTypeId, !currentState);
            syncStates(device);
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == brightnessActionTypeId) {
            device->setStateValue(brightnessStateTypeId, action.param("brightness").value().toInt());
            syncStates(device);
            return DeviceManager::DeviceErrorNoError;
        }
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

