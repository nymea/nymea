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

#include "scriptstate.h"

#include <QColor>
#include <qqml.h>
#include <QQmlEngine>
#include <QQmlContext>

#include "logging/logengine.h"

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)

namespace nymeaserver {
namespace scriptengine {

ScriptState::ScriptState(QObject *parent) : QObject(parent)
{

}

void ScriptState::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
    connect(m_thingManager, &ThingManager::thingStateChanged, this, &ScriptState::onThingStateChanged);

    connect(m_thingManager, &ThingManager::thingAdded, this, [this](Thing *newThing){
        if (newThing->id() == ThingId(m_thingId)) {
            qCDebug(dcScriptEngine()) << "Thing" << newThing->name() << "appeared in system";
            connectToThing();
        }
    });

    m_scriptId = qmlEngine(this)->contextForObject(this)->contextProperty("scriptId").toUuid();
    m_logger = qmlEngine(this)->contextForObject(this)->contextProperty("logger").value<Logger*>();
}

void ScriptState::componentComplete()
{

}

QString ScriptState::thingId() const
{
    return m_thingId;
}

void ScriptState::setThingId(const QString &thingId)
{
    if (m_thingId != thingId) {
        m_thingId = thingId;
        emit thingIdChanged();
        store();
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }

        connectToThing();
    }
}

QString ScriptState::stateTypeId() const
{
    return m_stateTypeId;
}

void ScriptState::setStateTypeId(const QString &stateTypeId)
{
    if (m_stateTypeId != stateTypeId) {
        m_stateTypeId = stateTypeId;
        emit stateTypeChanged();
        store();
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }
    }
}

QString ScriptState::stateName() const
{
    return m_stateName;
}

void ScriptState::setStateName(const QString &stateName)
{
    if (m_stateName != stateName) {
        m_stateName = stateName;
        emit stateTypeChanged();
        store();
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }
    }
}

QVariant ScriptState::value() const
{
    Thing* thing = m_thingManager->findConfiguredThing(ThingId(m_thingId));
    if (!thing) {
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "No thing with Id" << m_thingId << "found.";
        return QVariant();
    }
    StateTypeId stateTypeId = StateTypeId(m_stateTypeId);
    if (stateTypeId.isNull()) {
        stateTypeId = thing->thingClass().stateTypes().findByName(m_stateName).id();
    }

    return thing->stateValue(stateTypeId);
}

void ScriptState::setValue(const QVariant &value)
{
    if (m_pendingActionInfo) {
        m_valueCache = value;
        return;
    }

    Thing* thing = m_thingManager->findConfiguredThing(ThingId(m_thingId));
    if (!thing) {
        m_valueCache = value;
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "No thing with Id" << m_thingId << "found.";
        return;
    }

    if (thing->setupStatus() != Thing::ThingSetupStatusComplete) {
        m_valueCache = value;
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Thing" << thing->name() << "(" << m_thingId << ") is not ready yet";
        return;
    }

    ActionTypeId actionTypeId;
    if (!m_stateTypeId.isNull()) {
        actionTypeId = thing->thingClass().stateTypes().findById(StateTypeId(m_stateTypeId)).id();
        if (actionTypeId.isNull()) {
            QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Thing" << thing->name() << "does not have a state with type id" << m_stateTypeId;
        }
    }
    if (actionTypeId.isNull()) {
        actionTypeId = thing->thingClass().stateTypes().findByName(stateName()).id();
        if (actionTypeId.isNull()) {
            QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Thing" << thing->name() << "does not have a state named" << m_stateName;
        }
    }

    if (actionTypeId.isNull()) {
        m_valueCache = value;
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Either stateTypeId or stateName is required to be valid.";
        return;
    }

    Action action(ActionTypeId(actionTypeId), ThingId(m_thingId), Action::TriggeredByScript);
    ParamList params = ParamList() << Param(ParamTypeId(actionTypeId), value);
    action.setParams(params);

    qCDebug(dcScriptEngine()) << "Executing action on" << thing->name();

    m_valueCache = QVariant();
    m_pendingActionInfo = m_thingManager->executeAction(action);
    connect(m_pendingActionInfo, &ThingActionInfo::finished, this, [this, thing, actionTypeId](){

        ActionType actionType = thing->thingClass().actionTypes().findById(actionTypeId);
        m_logger->log({m_scriptId.toString(), "action"}, {
                          {"thingId", thing->id()},
                          {"action", actionType.name()},
                          {"status", QMetaEnum::fromType<Thing::ThingError>().valueToKey(m_pendingActionInfo->status())}
                      });

        m_pendingActionInfo = nullptr;
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }

    });
}

QVariant ScriptState::minimumValue() const
{
    Thing *thing = m_thingManager->configuredThings().findById(ThingId(m_thingId));
    if (!thing) {
        return QVariant();
    }
    StateType stateType = thing->thingClass().stateTypes().findById(StateTypeId(m_stateTypeId));
    if (stateType.id().isNull()) {
        stateType = thing->thingClass().stateTypes().findByName(m_stateName);
    }
    return stateType.minValue();
}

QVariant ScriptState::maximumValue() const
{
    Thing *thing = m_thingManager->configuredThings().findById(ThingId(m_thingId));
    if (!thing) {
        return QVariant();
    }
    StateType stateType = thing->thingClass().stateTypes().findById(StateTypeId(m_stateTypeId));
    if (stateType.id().isNull()) {
        stateType = thing->thingClass().stateTypes().findByName(m_stateName);
    }
    return stateType.maxValue();
}

void ScriptState::store()
{
    m_valueStore = value();
}

void ScriptState::restore()
{
    setValue(m_valueStore);
}

void ScriptState::onThingStateChanged(Thing *thing, const StateTypeId &stateTypeId)
{
    if (thing->id() != ThingId(m_thingId)) {
        return;
    }
    StateTypeId localStateTypeId = StateTypeId(m_stateTypeId);
    if (localStateTypeId.isNull()) {
        localStateTypeId = thing->thingClass().stateTypes().findByName(m_stateName).id();
    }
    if (localStateTypeId.isNull()) {
        return;
    }
    if (stateTypeId == localStateTypeId) {
        emit valueChanged();
    }
}

void ScriptState::connectToThing()
{
    if (m_connection) {
        disconnect(m_connection);
    }
    Thing *thing = m_thingManager->findConfiguredThing(ThingId(m_thingId));
    if (!thing) {
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "No thing with Id" << m_thingId << "found (yet - it may still appear).";
        return;
    }

    if (thing->setupStatus() == Thing::ThingSetupStatusComplete) {
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }
    } else {
        QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Setup for thing" << thing->name() << "(" << m_thingId << ") is not completed yet.";
    }

    m_connection = connect(thing, &Thing::setupStatusChanged, this, [this, thing](){
        if (thing->setupStatus() == Thing::ThingSetupStatusComplete) {
            QMessageLogger(qmlEngine(this)->contextForObject(this)->baseUrl().toString().toUtf8(), 0, "", "qml").warning() << "Setup for" << thing->name() << "(" << m_thingId << ") completed.";
            if (!m_valueCache.isNull()) {
                setValue(m_valueCache);
            }
        }
    });
}

}
}
