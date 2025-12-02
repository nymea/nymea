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

#include "scriptthing.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QJsonDocument>
#include <QQmlContext>
#include <QRegularExpression>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)

namespace nymeaserver {
namespace scriptengine {

ScriptThing::ScriptThing(QObject *parent)
    : QObject{parent}
{

}

ScriptThing::ScriptThing(ThingManager *thingManager, QObject *parent)
    : QObject{parent}
{
    init(thingManager);
}

void ScriptThing::classBegin()
{
    init(reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong()));
}

void ScriptThing::componentComplete()
{

}

QString ScriptThing::thingId() const
{
    return m_thingId.toString();
}

void ScriptThing::setThingId(const QString &thingId)
{
    if (m_thingId != ThingId(thingId)) {
        m_thingId = ThingId(thingId);
        emit thingIdChanged();
        emit nameChanged();
        connectToThing();
    }
}

QString ScriptThing::name() const
{
    Thing *thing = m_thingManager->findConfiguredThing(m_thingId);
    if (!thing) {
        return QString();
    }
    return thing->name();
}

QVariant ScriptThing::stateValue(const QString &stateName) const
{
    Thing *thing = m_thingManager->findConfiguredThing(m_thingId);
    if (!thing) {
        return QVariant();
    }
    return thing->stateValue(stateName);
}

void ScriptThing::setStateValue(const QString &stateName, const QVariant &value)
{
    executeAction(stateName, {{stateName, value}});
}

void ScriptThing::executeAction(const QString &actionName, const QVariantMap &params)
{
    Thing *thing = m_thingManager->findConfiguredThing(m_thingId);
    if (!thing) {
        return;
    }

    ActionType actionType = thing->thingClass().actionTypes().findByName(actionName);
    if (actionType.id().isNull()) { // Try to find by id for now, for compatiblity sake
        actionType = thing->thingClass().actionTypes().findById(QUuid(actionName));
    }
    if (actionType.id().isNull()) {
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Thing" << thing->name() << "does not have action" << actionName;
        return;
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
    qCDebug(dcScriptEngine()) << "Executing action:" << action.thingId().toString() << action.actionTypeId().toString() << action.params();
    m_thingManager->executeAction(action);

}

void ScriptThing::init(ThingManager *thingManager)
{
    m_thingManager = thingManager;
    connect(m_thingManager, &ThingManager::thingAdded, this, [this](Thing *newThing){
        if (newThing->id() == m_thingId) {
            qCDebug(dcScriptEngine()) << "Thing" << newThing->name() << "appeared in system";
            connectToThing();
        }
    });
    connect(m_thingManager, &ThingManager::thingStateChanged, this, [=](Thing *thing, const StateTypeId &stateTypeId, const QVariant &value, const QVariant &minValue, const QVariant &maxValue){
        Q_UNUSED(minValue)
        Q_UNUSED(maxValue)
        if (m_thingId != thing->id()) {
            return;
        }
        emit stateValueChanged(thing->thingClass().getStateType(stateTypeId).name(), value);
    });
    connect(m_thingManager, &ThingManager::eventTriggered, this, [=](const Event &event){
        if (m_thingId != event.thingId()) {
            return;
        }

        Thing *thing = m_thingManager->findConfiguredThing(event.thingId());
        QVariantMap params;
        foreach (const Param &param, event.params()) {
            params.insert(param.paramTypeId().toString().remove(QRegularExpression("[{}]")), param.value().toByteArray());
            QString paramName = thing->thingClass().eventTypes().findById(event.eventTypeId()).paramTypes().findById(param.paramTypeId()).name();
            params.insert(paramName, param.value().toByteArray());
        }

        // Note: Explicitly convert the params to a Json document because auto-casting from QVariantMap to the JS engine might drop some values.
        emit eventTriggered(thing->thingClass().eventTypes().findById(event.eventTypeId()).name(), QJsonDocument::fromVariant(params).toVariant().toMap());
    });
    connect(m_thingManager, &ThingManager::actionExecuted, this, [=](const Action &action, Thing::ThingError status){
        if (m_thingId != action.thingId()) {
            return;
        }

        Thing *thing = m_thingManager->findConfiguredThing(action.thingId());
        QVariantMap params;
        foreach (const Param &param, action.params()) {
            params.insert(param.paramTypeId().toString().remove(QRegularExpression("[{}]")), param.value().toByteArray());
            QString paramName = thing->thingClass().actionTypes().findById(action.actionTypeId()).paramTypes().findById(param.paramTypeId()).name();
            params.insert(paramName, param.value().toByteArray());
        }

        // Note: Explicitly convert the params to a Json document because auto-casting from QVariantMap to the JS engine might drop some values.
        emit actionExecuted(thing->thingClass().actionTypes().findById(action.actionTypeId()).name(), QJsonDocument::fromVariant(params).toVariant().toMap(), status, action.triggeredBy());
    });
}

void ScriptThing::connectToThing()
{
    disconnect(m_nameConnection);

    Thing *thing = m_thingManager->findConfiguredThing(m_thingId);
    if (!thing) {
        qCDebug(dcScriptEngine()) << "Can't find thing with id" << m_thingId.toString() << "(yet)";
        return;
    }


    m_nameConnection = connect(thing, &Thing::nameChanged, this, [this, thing](){
        if (thing->setupStatus() == Thing::ThingSetupStatusComplete) {
            qCDebug(dcScriptEngine()) << "Thing setup for" << thing->name() << "completed";
            emit nameChanged();
        }
    });
}

}
}
