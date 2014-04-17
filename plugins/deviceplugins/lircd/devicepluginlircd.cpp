#include "devicepluginlircd.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include "lircdclient.h"

#include <QDebug>
#include <QStringList>

VendorId lircdVendorId = VendorId("9a53049c-8828-4b87-b3f6-7bc7708196cd");

PluginId lircdPluginUuid = PluginId("075f734f-4d76-4ce3-9ef8-34c212285676");
DeviceClassId lircdDeviceClassId = DeviceClassId("5c2bc4cd-ba6c-4052-b6cd-1db83323ea22");
EventTypeId LircKeypressEventTypeId = EventTypeId("8711471a-fa0e-410b-b174-dfc3d2aeffb1");

DevicePluginLircd::DevicePluginLircd()
{
    m_lircClient = new LircClient(this);

    m_lircClient->connect();
    connect(m_lircClient, &LircClient::buttonPressed, this, &DevicePluginLircd::buttonPressed);
}

QList<Vendor> DevicePluginLircd::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor guh(lircdVendorId, "Lircd");
    ret.append(guh);
    return ret;
}

QList<DeviceClass> DevicePluginLircd::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassLircd(pluginId(), lircdVendorId, lircdDeviceClassId);
    deviceClassLircd.setName("IR Receiver");

    QVariantList params;
    QVariantMap remoteNameParam;
    remoteNameParam.insert("name", "remoteName");
    remoteNameParam.insert("type", "string");
    params.append(remoteNameParam);
    deviceClassLircd.setParams(params);
    
    // TODO: find a way to load this stuff from a json file, really!
    // Ideally that file can be generated from /usr/share/lirc/remotes/*
    // Note that the IDs need to be kept static!
    QVariantList repeatParam;
    QVariantMap repeatParamMap;
    repeatParamMap.insert("name", "repeat");
    repeatParamMap.insert("type", "int");
    repeatParam.append(repeatParamMap);

    QList<EventType> events;
    EventType powerButton(EventTypeId("d62d779f-e5c6-4767-98e6-efe9c062b662"));
    powerButton.setName("Power");
    powerButton.setParameters(repeatParam);
    events.append(powerButton);
    EventType yellowButton(EventTypeId("3313f62e-ea20-47f5-85af-28897d6ac440"));
    yellowButton.setName("Yellow");
    yellowButton.setParameters(repeatParam);
    events.append(yellowButton);
    EventType blueButton(EventTypeId("9a395d93-e482-4fa2-b4bc-e60bb4bf8652"));
    blueButton.setName("Blue");
    blueButton.setParameters(repeatParam);
    events.append(blueButton);
    EventType greenButton(EventTypeId("e8aaf18e-dc11-40da-980d-4eec42c58267"));
    greenButton.setName("Green");
    greenButton.setParameters(repeatParam);
    events.append(greenButton);
    EventType redButton(EventTypeId("b8518755-55a0-4cd4-8856-1680848edcb7"));
    redButton.setName("Red");
    redButton.setParameters(repeatParam);
    events.append(redButton);

    deviceClassLircd.setEvents(events);

    ret.append(deviceClassLircd);

    return ret;
}

DeviceManager::HardwareResources DevicePluginLircd::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

QString DevicePluginLircd::pluginName() const
{
    return "Lircd receiver";
}

PluginId DevicePluginLircd::pluginId() const
{
    return lircdPluginUuid;
}

void DevicePluginLircd::buttonPressed(const QString &remoteName, const QString &buttonName, int repeat)
{
    Device *remote = nullptr;
    QList<Device*> configuredRemotes = deviceManager()->findConfiguredDevices(lircdDeviceClassId);
    foreach (Device *device, configuredRemotes) {
        if (device->params().value("remoteName").toString() == remoteName) {
            remote = device;
            break;
        }
    }
    if (!remote) {
        qDebug() << "Unhandled remote" << remoteName << buttonName;
        return;
    }

    qDebug() << "found remote" << remoteName << supportedDevices().first().events().count();
    foreach (const EventType &eventType, supportedDevices().first().events()) {
        if (eventType.name() == buttonName) {
            QVariantMap param;
            param.insert("repeat", repeat);
            Event event(eventType.id(), remote->id(), param);
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
