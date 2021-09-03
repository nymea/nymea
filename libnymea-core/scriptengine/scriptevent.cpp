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

#include "scriptevent.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QJsonDocument>

namespace nymeaserver {

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

    if (!m_eventTypeId.isEmpty() && event.eventTypeId() != m_eventTypeId) {
        return;
    }

    Thing *thing = m_thingManager->findConfiguredThing(event.thingId());
    if (!m_eventName.isEmpty() && thing->thingClass().eventTypes().findByName(m_eventName).id() != event.eventTypeId()) {
        return;
    }

    QVariantMap params;
    foreach (const Param &param, event.params()) {
        params.insert(param.paramTypeId().toString().remove(QRegExp("[{}]")), param.value().toByteArray());
        QString paramName = thing->thingClass().eventTypes().findById(event.eventTypeId()).paramTypes().findById(param.paramTypeId()).name();
        params.insert(paramName, param.value().toByteArray());
    }

    // Note: Explicitly convert the params to a Json document because auto-casting from QVariantMap to the JS engine might drop some values.
    emit triggered(QJsonDocument::fromVariant(params).toVariant().toMap());
}

}

#endif // WITH_QML
