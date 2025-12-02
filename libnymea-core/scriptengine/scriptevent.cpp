// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "scriptevent.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QJsonDocument>
#include <QRegularExpression>

namespace nymeaserver {
namespace scriptengine {

ScriptEvent::ScriptEvent(QObject *parent) : QObject(parent)
{
}

void ScriptEvent::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
    connect(m_thingManager, &ThingManager::eventTriggered, this, &ScriptEvent::onEventTriggered);
}

void ScriptEvent::componentComplete()
{

}

QString ScriptEvent::thingId() const
{
    return m_thingId;
}

void ScriptEvent::setThingId(const QString &thingId)
{
    if (m_thingId != thingId) {
        m_thingId = thingId;
        emit thingIdChanged();
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
    if (ThingId(m_thingId) != event.thingId()) {
        return;
    }

    if (!m_eventTypeId.isEmpty() && event.eventTypeId() != EventTypeId(m_eventTypeId)) {
        return;
    }

    Thing *thing = m_thingManager->findConfiguredThing(event.thingId());
    if (!m_eventName.isEmpty() && thing->thingClass().eventTypes().findByName(m_eventName).id() != event.eventTypeId()) {
        return;
    }

    QVariantMap params;
    foreach (const Param &param, event.params()) {
        params.insert(param.paramTypeId().toString().remove(QRegularExpression("[{}]")), param.value().toByteArray());
        QString paramName = thing->thingClass().eventTypes().findById(event.eventTypeId()).paramTypes().findById(param.paramTypeId()).name();
        params.insert(paramName, param.value().toByteArray());
    }

    // Note: Explicitly convert the params to a Json document because auto-casting from QVariantMap to the JS engine might drop some values.
    emit triggered(QJsonDocument::fromVariant(params).toVariant().toMap());
}

}
}
