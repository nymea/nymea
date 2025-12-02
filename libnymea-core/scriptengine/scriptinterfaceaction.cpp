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

#include "scriptinterfaceaction.h"

#include "integrations/thingmanager.h"
#include "types/action.h"

#include <QQmlEngine>
#include <qqml.h>
#include <QQmlContext>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)

namespace nymeaserver {
namespace scriptengine {

ScriptInterfaceAction::ScriptInterfaceAction(QObject *parent) : QObject(parent)
{

}

void ScriptInterfaceAction::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
}

void ScriptInterfaceAction::componentComplete()
{

}

QString ScriptInterfaceAction::interfaceName() const
{
    return m_interfaceName;
}

void ScriptInterfaceAction::setInterfaceName(const QString &interfaceName)
{
    if (m_interfaceName != interfaceName) {
        m_interfaceName = interfaceName;
        emit interfaceNameChanged();
    }
}

QString ScriptInterfaceAction::actionName() const
{
    return m_actionName;
}

void ScriptInterfaceAction::setActionName(const QString &actionName)
{
    if (m_actionName != actionName) {
        m_actionName = actionName;
        emit actionNameChanged();
    }
}

void ScriptInterfaceAction::execute(const QVariantMap &params)
{
    Things things;
    if (!m_interfaceName.isEmpty()) {
        foreach (Thing *thing, m_thingManager->configuredThings()) {
            if (thing->thingClass().interfaces().contains(m_interfaceName)) {
                things.append(thing);
            }
        }
    }
    if (things.isEmpty()) {
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "No things matching by interface" << m_interfaceName;
        return;
    }

    foreach (Thing *thing, things) {
        ActionType actionType = thing->thingClass().actionTypes().findByName(m_actionName);
        if (actionType.id().isNull()) {
            QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Thing" << thing->name() << "does not have action" << m_actionName;
            continue;
        }
        Action action(actionType.id(), thing->id(), Action::TriggeredByScript);
        ParamList paramList;
        foreach (const QString &paramNameOrId, params.keys()) {
            ParamType paramType;
            if (!ParamTypeId(paramNameOrId).isNull()) {
                paramType = actionType.paramTypes().findById(ParamTypeId(paramNameOrId));
            } else {
                paramType = actionType.paramTypes().findByName(paramNameOrId);
            }
            if (paramType.id().isNull()) {
                QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Invalid param id or name";
                continue;
            }
            paramList << Param(paramType.id(), params.value(paramNameOrId));
        }
        action.setParams(paramList);
        qCDebug(dcScriptEngine()) << "Executing action:" << action.thingId() << action.actionTypeId() << action.params();
        m_thingManager->executeAction(action);
    }
}

}
}
