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

#include "devicepluginlircd.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include "lircdclient.h"

#include <QDebug>
#include <QStringList>

DeviceClassId lircdDeviceClassId = DeviceClassId("5c2bc4cd-ba6c-4052-b6cd-1db83323ea22");
EventTypeId LircKeypressEventTypeId = EventTypeId("8711471a-fa0e-410b-b174-dfc3d2aeffb1");

DevicePluginLircd::DevicePluginLircd()
{
    m_lircClient = new LircClient(this);

    m_lircClient->connect();
    connect(m_lircClient, &LircClient::buttonPressed, this, &DevicePluginLircd::buttonPressed);
}

//QList<DeviceClass> DevicePluginLircd::supportedDevices() const
//{
//    QList<DeviceClass> ret;

//    DeviceClass deviceClassLircd(pluginId(), supportedVendors().first().id(), lircdDeviceClassId);
//    deviceClassLircd.setName("IR Receiver");

//    QList<ParamType> params;
//    ParamType remoteNameParam("remoteName", QVariant::String);
//    params.append(remoteNameParam);
//    deviceClassLircd.setParamTypes(params);
    
//    // TODO: find a way to load this stuff from a json file, really!
//    // Ideally that file can be generated from /usr/share/lirc/remotes/*
//    // Note that the IDs need to be kept static!
//    QList<ParamType> repeatParam;
//    ParamType repeatParamMap("repeat", QVariant::Int);
//    repeatParam.append(repeatParamMap);

//    QList<EventType> events;
//    EventType powerButton(EventTypeId("d62d779f-e5c6-4767-98e6-efe9c062b662"));
//    powerButton.setName("Power");
//    powerButton.setParameters(repeatParam);
//    events.append(powerButton);
//    EventType yellowButton(EventTypeId("3313f62e-ea20-47f5-85af-28897d6ac440"));
//    yellowButton.setName("Yellow");
//    yellowButton.setParameters(repeatParam);
//    events.append(yellowButton);
//    EventType blueButton(EventTypeId("9a395d93-e482-4fa2-b4bc-e60bb4bf8652"));
//    blueButton.setName("Blue");
//    blueButton.setParameters(repeatParam);
//    events.append(blueButton);
//    EventType greenButton(EventTypeId("e8aaf18e-dc11-40da-980d-4eec42c58267"));
//    greenButton.setName("Green");
//    greenButton.setParameters(repeatParam);
//    events.append(greenButton);
//    EventType redButton(EventTypeId("b8518755-55a0-4cd4-8856-1680848edcb7"));
//    redButton.setName("Red");
//    redButton.setParameters(repeatParam);
//    events.append(redButton);

//    deviceClassLircd.setEventTypes(events);

//    ret.append(deviceClassLircd);

//    return ret;
//}

DeviceManager::HardwareResources DevicePluginLircd::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginLircd::buttonPressed(const QString &remoteName, const QString &buttonName, int repeat)
{
    Device *remote = nullptr;
    QList<Device*> configuredRemotes = deviceManager()->findConfiguredDevices(lircdDeviceClassId);
    foreach (Device *device, configuredRemotes) {
        if (device->paramValue("remoteName").toString() == remoteName) {
            remote = device;
            break;
        }
    }
    if (!remote) {
        qDebug() << "Unhandled remote" << remoteName << buttonName;
        return;
    }

    qDebug() << "found remote" << remoteName << supportedDevices().first().eventTypes().count();
    foreach (const EventType &eventType, supportedDevices().first().eventTypes()) {
        if (eventType.name() == buttonName) {
            ParamList params;
            Param param("repeat", repeat);
            params.append(param);
            Event event(eventType.id(), remote->id(), params);
            emitEvent(event);
        }
    }

}

//QVariantMap DevicePluginLircd::configuration() const
//{
//    return m_config;
//}

//void DevicePluginLircd::setConfiguration(const QVariantMap &configuration)
//{
//    m_config = configuration;
//}
