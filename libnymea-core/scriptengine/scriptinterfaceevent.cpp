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

#include "scriptinterfaceevent.h"

#include <qqml.h>
#include <QJsonDocument>
#include <QQmlEngine>
#include <QRegularExpression>

namespace nymeaserver {
namespace scriptengine {

ScriptInterfaceEvent::ScriptInterfaceEvent(QObject *parent)
    : QObject(parent)
{}

void ScriptInterfaceEvent::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager *>(qmlEngine(this)->property("thingManager").toULongLong());
    connect(m_thingManager, &ThingManager::eventTriggered, this, &ScriptInterfaceEvent::onEventTriggered);
}

void ScriptInterfaceEvent::componentComplete() {}

QString ScriptInterfaceEvent::interfaceName() const
{
    return m_interfaceName;
}

void ScriptInterfaceEvent::setInterfaceName(const QString &interfaceName)
{
    if (m_interfaceName != interfaceName) {
        m_interfaceName = interfaceName;
        emit interfaceNameChanged();
    }
}

QString ScriptInterfaceEvent::eventName() const
{
    return m_eventName;
}

void ScriptInterfaceEvent::setEventName(const QString &eventName)
{
    if (m_eventName != eventName) {
        m_eventName = eventName;
        emit eventNameChanged();
    }
}

void ScriptInterfaceEvent::onEventTriggered(const Event &event)
{
    Thing *thing = m_thingManager->findConfiguredThing(event.thingId());
    if (!thing->thingClass().interfaces().contains(m_interfaceName)) {
        return;
    }

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
    emit triggered(event.thingId().toString().remove(QRegularExpression("[{}]")), QJsonDocument::fromVariant(params).toVariant().toMap());
}

} // namespace scriptengine
} // namespace nymeaserver
