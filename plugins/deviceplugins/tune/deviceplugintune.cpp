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
    if (!m_manager->tuneAvailable()) {
        qWarning() << "WARNING: tune not connected!";
    }

    // check index position
    int position = device->paramValue("position").toInt();
    if (position >= myDevices().count()) {
        device->setParamValue("position", myDevices().count());
    } else {
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

    // todo
    if (device->deviceClassId() == todoDeviceClassId) {
        device->setName(device->paramValue("name").toString() + " (Todo)");
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
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
    QVariantList devices;
    foreach (Device* device, myDevices()) {
        qDebug() << "device id" << device->id();
        QVariantMap d;
        d.insert("name", device->paramValue("name"));
        d.insert("id", device->id());
        d.insert("deviceClassId", device->deviceClassId());
        d.insert("pos", device->paramValue("position"));
        d.insert("icon", device->paramValue("icon"));
        devices.append(d);
    }
    message.insert("devices", devices);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Compact);

    qDebug() << data;

    m_manager->sendData(data);

    return true;
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
    qDebug() << jsonDoc.toJson();

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
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // Todo
    if (device->deviceClassId() == todoDeviceClassId) {
        if (action.actionTypeId() == pressActionTypeId) {
            emit emitEvent(Event(pressedEventTypeId, device->id()));
            return DeviceManager::DeviceErrorNoError;
        }
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

