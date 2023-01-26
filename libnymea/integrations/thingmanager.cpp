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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "thingmanager.h"

/*!
    \class ThingManager
    \brief nymea's thing manager interface.

    \ingroup things
    \inmodule libnymea

    The ThingManager is the main entity managing all things in nymea.

    It is also responsible for loading Plugins and managing common hardware resources between
    \l{IntegrationPlugin}{integration plugins}.
*/

ThingManager::ThingManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<Param>();
    qRegisterMetaType<ParamList>();
    qRegisterMetaType<ParamTypeId>();
    qRegisterMetaType<ParamType>();
    qRegisterMetaType<ParamTypes>();
    qRegisterMetaType<StateTypeId>();
    qRegisterMetaType<StateType>();
    qRegisterMetaType<StateTypes>();
    qRegisterMetaType<EventTypeId>();
    qRegisterMetaType<EventType>();
    qRegisterMetaType<EventTypes>();
    qRegisterMetaType<ActionTypeId>();
    qRegisterMetaType<ActionType>();
    qRegisterMetaType<ActionTypes>();
    qRegisterMetaType<ThingClassId>();
    qRegisterMetaType<ThingClass>();
    qRegisterMetaType<ThingClasses>();
    qRegisterMetaType<ThingDescriptorId>();
    qRegisterMetaType<ThingDescriptor>();
    qRegisterMetaType<ThingDescriptors>();
    qRegisterMetaType<Thing::ThingError>();
}

/*! Connect two states.
    When two states are connected, any state changes will be synced between those. A connection
    is made from an \a inputThing and its \a inputState to an \a outputThing and its \a outputState.
    Whenever the input state changes, the output state is set accordingly. If the input state is
    writable, the connection will be bidirectional, that is, a change of the output state will also
    reflect on the input state.
    Connections can be logically inverted.
    Connections need to be compatible. This means, only states which have a defined ioState of type "input"
    can be connected to states which habe a defined ioState of type "output". Additionally, the digital/analog
    type needs to match. In other words, states with ioType "digitalInput" can be connected to states with ioType
    "digitaOutput" and states with ioType "analogInput" can be connected to states with ioType "analogOutput".
 */
IOConnectionResult ThingManager::connectIO(const ThingId &inputThingId, const QString &inputState, const ThingId &outputThingId, const QString &outputState, bool inverted)
{
    IOConnection connection(IOConnectionId::createIOConnectionId(), inputThingId, inputState, outputThingId, outputState, inverted);
    return connectIO(connection);
}
