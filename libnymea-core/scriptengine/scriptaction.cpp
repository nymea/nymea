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
    m_deviceManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("deviceManager").toULongLong());
}

void ScriptAction::componentComplete()
{

}

QString ScriptAction::deviceId() const
{
    return m_thingId;
}

void ScriptAction::setDeviceId(const QString &deviceId)
{
    if (m_thingId != deviceId) {
        m_thingId = deviceId;
        emit deviceIdChanged();
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
    Thing *thing = m_deviceManager->configuredThings().findById(ThingId(m_thingId));
    if (!thing) {
        qCWarning(dcScriptEngine) << "No device with id" << m_thingId;
        return;
    }
    ActionType actionType;
    if (!ActionTypeId(m_actionTypeId).isNull()) {
        actionType = thing->thingClass().actionTypes().findById(ActionTypeId(m_actionTypeId));
    } else {
        actionType = thing->thingClass().actionTypes().findByName(m_actionName);
    }
    if (actionType.id().isNull()) {
        qCWarning(dcScriptEngine()) << "Either a valid actionTypeId or actionName is required";
        return;
    }
    Action action;
    action.setActionTypeId(actionType.id());
    action.setThingId(ThingId(m_thingId));
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
    m_deviceManager->executeAction(action);
}

}
