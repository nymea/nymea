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

ThingManager::ThingManager(QObject *parent)
    : QObject(parent)
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
IOConnectionResult ThingManager::connectIO(const ThingId &inputThing, const StateTypeId &inputState, const ThingId &outputThing, const StateTypeId &outputState, bool inverted)
{
    IOConnection connection(IOConnectionId::createIOConnectionId(), inputThing, inputState, outputThing, outputState, inverted);
    return connectIO(connection);
}
