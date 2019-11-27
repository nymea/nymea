#include "scriptevent.h"

#include <qqml.h>
#include <QQmlEngine>

namespace nymeaserver {

ScriptEvent::ScriptEvent(QObject *parent) : QObject(parent)
{
}

void ScriptEvent::classBegin()
{
    m_deviceManager = reinterpret_cast<DeviceManager*>(qmlEngine(this)->property("deviceManager").toULongLong());
    connect(m_deviceManager, &DeviceManager::eventTriggered, this, &ScriptEvent::onEventTriggered);
}

void ScriptEvent::componentComplete()
{

}

QString ScriptEvent::deviceId() const
{
    return m_deviceId;
}

void ScriptEvent::setDeviceId(const QString &deviceId)
{
    if (m_deviceId != deviceId) {
        m_deviceId = deviceId;
        emit deviceIdChanged();
    }
}

QString ScriptEvent::eventTypeId() const
{
    return m_eventTypeId;
}

void ScriptEvent::setEventTypeId(const QString &eventTypeId)
{
    if (m_eventTypeId != eventTypeId) {
        m_eventTypeId = eventTypeId;
        emit eventTypeIdChanged();
    }
}

QString ScriptEvent::eventName() const
{
    return m_eventName;
}

void ScriptEvent::setEventName(const QString &eventName)
{
    if (m_eventName != eventName) {
        m_eventName = eventName;
        emit eventNameChanged();
    }
}

void ScriptEvent::onEventTriggered(const Event &event)
{
    if (DeviceId(m_deviceId) != event.deviceId()) {
        return;
    }

    if (!m_eventTypeId.isEmpty() && event.eventTypeId() != m_eventTypeId) {
        return;
    }

    Device *device = m_deviceManager->findConfiguredDevice(event.deviceId());
    if (!m_eventName.isEmpty() && device->deviceClass().eventTypes().findByName(m_eventName).id() != event.eventTypeId()) {
        return;
    }

    emit triggered();
}

}

