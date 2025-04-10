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

#include "scriptinterfaceevent.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QJsonDocument>

namespace nymeaserver {
namespace scriptengine {

ScriptInterfaceEvent::ScriptInterfaceEvent(QObject *parent) : QObject(parent)
{
}

void ScriptInterfaceEvent::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
    connect(m_thingManager, &ThingManager::eventTriggered, this, &ScriptInterfaceEvent::onEventTriggered);
}

void ScriptInterfaceEvent::componentComplete()
{

}

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

}
}
