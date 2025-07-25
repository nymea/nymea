/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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
