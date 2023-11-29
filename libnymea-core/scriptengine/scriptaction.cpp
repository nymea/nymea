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
#include "logging/logengine.h"

#include <QQmlEngine>
#include <qqml.h>
#include <QQmlContext>
#include <QJsonDocument>

#include <QMessageLogger>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)

namespace nymeaserver {
namespace scriptengine {


ScriptAction::ScriptAction(QObject *parent) : QObject(parent)
{

}

void ScriptAction::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());

    m_scriptId = qmlEngine(this)->contextForObject(this)->contextProperty("scriptId").toUuid();
    m_logger = qmlEngine(this)->contextForObject(this)->contextProperty("logger").value<Logger*>();

    connect(m_thingManager, &ThingManager::actionExecuted, this, [=](const Action &action, Thing::ThingError status){
        if (ThingId(m_thingId) != action.thingId()) {
            return;
        }

        Thing *thing = m_thingManager->findConfiguredThing(ThingId(m_thingId));
        if (!thing) {
            return;
        }

        ActionTypeId ourActionTypeId = ActionTypeId(m_actionTypeId);
        if (ourActionTypeId.isNull()) {
            ourActionTypeId = thing->thingClass().actionTypes().findByName(m_actionName).id();
        }
        if (ourActionTypeId.isNull() || action.actionTypeId() != ourActionTypeId) {
            return;
        }

        QVariantMap params;
        foreach (const Param &param, action.params()) {
            params.insert(param.paramTypeId().toString().remove(QRegExp("[{}]")), param.value().toByteArray());
            QString paramName = thing->thingClass().actionTypes().findById(action.actionTypeId()).paramTypes().findById(param.paramTypeId()).name();
            params.insert(paramName, param.value().toByteArray());
        }

        // Note: Explicitly convert the params to a Json document because auto-casting from QVariantMap to the JS engine might drop some values.
        emit executed(QJsonDocument::fromVariant(params).toVariant().toMap(), status, action.triggeredBy());
    });
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
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "No things matching id" << m_thingId << "or interface" << m_interfaceName;
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
            QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Thing" << thing->name() << "does not have actionTypeId" << m_actionTypeId << "or actionName" << m_actionName;
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
                QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Invalid param id or name:" << paramNameOrId;
                continue;
            }
            paramList << Param(paramType.id(), params.value(paramNameOrId));
        }
        action.setParams(paramList);
        qCDebug(dcScriptEngine()) << "Executing action:" << action.thingId() << action.actionTypeId() << action.params();
        ThingActionInfo *actionInfo = m_thingManager->executeAction(action);
        connect(actionInfo, &ThingActionInfo::finished, this, [this, actionInfo, thing, action](){
            ActionType actionType = thing->thingClass().actionTypes().findById(action.actionTypeId());
            m_logger->log({m_scriptId.toString(), "action"}, {
                              {"thingId", thing->id()},
                              {"action", actionType.name()},
                              {"status", QMetaEnum::fromType<Thing::ThingError>().valueToKey(actionInfo->status())}
                          });

        });
    }
}

}
}
