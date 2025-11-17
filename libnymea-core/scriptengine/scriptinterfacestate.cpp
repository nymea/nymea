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

#include "scriptinterfacestate.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QJsonDocument>
#include <QRegularExpression>

namespace nymeaserver {
namespace scriptengine {

ScriptInterfaceState::ScriptInterfaceState(QObject *parent) : QObject(parent)
{
}

void ScriptInterfaceState::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
    connect(m_thingManager, &ThingManager::thingStateChanged, this, &ScriptInterfaceState::onStateChanged);
}

void ScriptInterfaceState::componentComplete()
{

}

QString ScriptInterfaceState::interfaceName() const
{
    return m_interfaceName;
}

void ScriptInterfaceState::setInterfaceName(const QString &interfaceName)
{
    if (m_interfaceName != interfaceName) {
        m_interfaceName = interfaceName;
        emit interfaceNameChanged();
    }
}

QString ScriptInterfaceState::stateName() const
{
    return m_stateName;
}

void ScriptInterfaceState::setStateName(const QString &stateName)
{
    if (m_stateName != stateName) {
        m_stateName = stateName;
        emit stateNameChanged();
    }
}

void ScriptInterfaceState::onStateChanged(Thing *thing, const StateTypeId &stateTypeId, const QVariant &value)
{
    if (!thing->thingClass().interfaces().contains(m_interfaceName)) {
        return;
    }

    if (!m_stateName.isEmpty() && thing->thingClass().stateTypes().findByName(m_stateName).id() != stateTypeId) {
        return;
    }

    emit stateChanged(thing->id().toString().remove(QRegularExpression("[{}]")), value);
}

}
}
