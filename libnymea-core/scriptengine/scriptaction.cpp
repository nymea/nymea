/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef WITH_QML

#include "scriptaction.h"

#include "integrations/thingmanager.h"
#include "types/action.h"

#include <QQmlEngine>
#include <qqml.h>

#include "loggingcategories.h"

namespace nymeaserver {

ScriptAction::ScriptAction(QObject *parent) : QObject(parent)
{

}

void ScriptAction::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
}

void ScriptAction::componentComplete()
{

}

QString ScriptAction::thingId() const
{
    return m_thingId;
}

void ScriptAction::setThingId(const QString &thingId)
{
    if (m_thingId != thingId) {
        m_thingId = thingId;
        emit thingIdChanged();
    }
}

QString ScriptAction::interfaceName() const
{
    return m_interfaceName;
}

void ScriptAction::setInterfaceName(const QString &interfaceName)
{
    if (m_interfaceName != interfaceName) {
        m_interfaceName = interfaceName;
        emit interfaceNameChanged();
    }
}

QString ScriptAction::actionTypeId() const
{
    return m_actionTypeId;
}

void ScriptAction::setActionTypeId(const QString &actionTypeId)
{
    if (m_actionTypeId != actionTypeId) {
        m_actionTypeId = actionTypeId;
        emit actionTypeIdChanged();
    }
}

QString ScriptAction::actionName() const
{
    return m_actionName;
}

void ScriptAction::setActionName(const QString &actionName)
{
    if (m_actionName != actionName) {
        m_actionName = actionName;
        emit actionNameChanged();
    }
}

void ScriptAction::execute(const QVariantMap &params)
{
    Things things;
    if (m_thingId.isEmpty() && !m_interfaceName.isEmpty()) {
        foreach (Thing *thing, m_thingManager->configuredThings()) {
            if (thing->thingClass().interfaces().contains(m_interfaceName)) {
                things.append(thing);
            }
        }
    }
    Thing *thing = m_thingManager->configuredThings().findById(ThingId(m_thingId));
    if (thing && !things.contains(thing)) {
        things.append(thing);
    }
    if (things.isEmpty()) {
        qCWarning(dcScriptEngine) << "No things matching by id" << m_thingId << "and interface" << m_interfaceName;
        return;
    }

    foreach (Thing *thing, things) {
        ActionType actionType;
        if (!ActionTypeId(m_actionTypeId).isNull()) {
            actionType = thing->thingClass().actionTypes().findById(ActionTypeId(m_actionTypeId));
        } else {
            actionType = thing->thingClass().actionTypes().findByName(m_actionName);
        }
        if (actionType.id().isNull()) {
            qCWarning(dcScriptEngine()) << "Thing" << thing->name() << "does not have actionTypeId" << m_actionTypeId << "or actionName" << m_actionName;
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
                qCWarning(dcScriptEngine()) << "Invalid param id or name";
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

#endif // WITH_QML
