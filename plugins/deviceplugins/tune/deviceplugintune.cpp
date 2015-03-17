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

    m_manager->start();
}

DeviceManager::HardwareResources DevicePluginTune::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginTune::setupDevice(Device *device)
{
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

    // mood
    if (device->deviceClassId() == moodDeviceClassId) {
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
    int position = device->paramValue("position").toInt();

    foreach (Device *d, myDevices()) {
        int currentPosition = d->paramValue("position").toInt();
        if (currentPosition >= position ) {
            d->setParamValue("position", currentPosition - 1);
        }
    }
    sync();
}

bool DevicePluginTune::sync()
{
    // sync with devices with tune
    if (!m_manager->tuneAvailable()) {
        return false;
    }

    QVariantMap message;
    QVariantList moods;
    foreach (Device* device, myDevices()) {
        if (device->deviceClassId() == moodDeviceClassId) {
            QVariantMap mood;
            mood.insert("name", device->paramValue("name"));
            mood.insert("deviceId", device->id());
            mood.insert("deviceClassId", device->deviceClassId().toString());
            mood.insert("position", device->paramValue("position"));
            mood.insert("icon", device->paramValue("icon"));
            QVariantMap states;
            states.insert("value", device->stateValue(valueStateTypeId).toInt());
            states.insert("active", device->stateValue(activeStateTypeId).toBool());
            mood.insert("states", states);
            moods.append(mood);
        }
    }

    message.insert("method", "Items.Sync");
    message.insert("moods", moods);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    //qDebug() << data;

    m_manager->sendData(data);
    return true;
}

void DevicePluginTune::syncStates(Device *device)
{
    QVariantMap message;
    QVariantMap mood;
    QVariantMap states;
    states.insert("value", device->stateValue(valueStateTypeId).toInt());
    states.insert("active", device->stateValue(activeStateTypeId).toBool());
    mood.insert("states", states);
    mood.insert("deviceId", device->id());
    mood.insert("position", device->paramValue("position"));
    message.insert("method", "Items.SyncStates");
    message.insert("mood", mood);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    //qDebug() << jsonDoc.toJson();

    m_manager->sendData(data);

}

void DevicePluginTune::tuneConnectionStatusChanged(const bool &connected)
{
    if (connected) {
        sync();
    }
}

void DevicePluginTune::tuneDataAvailable(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << data << ":" << error.errorString();
    }

    QVariantMap message = jsonDoc.toVariant().toMap();
    if (message.contains("method")) {
        if (message.value("method").toString() == "Items.SyncStates") {
            QVariantMap mood = message.value("mood").toMap();


            foreach (Device *device, myDevices()) {
                if (device->id() == DeviceId(mood.value("deviceId").toString())) {
                    device->setStateValue(activeStateTypeId, mood.value("states").toMap().value("active"));
                    device->setStateValue(valueStateTypeId, mood.value("states").toMap().value("value"));
                    return;
                }
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
        if (action.actionTypeId() == toggleActionTypeId) {
            bool currentState = device->stateValue(activeStateTypeId).toBool();
            device->setStateValue(activeStateTypeId, !currentState);
            syncStates(device);
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == setValueActionTypeId) {
            device->setStateValue(valueStateTypeId, action.param("percentage").value().toInt());
            syncStates(device);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

