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

/*!
  \class IOConnection
  \brief The IOConnection class stores information about generic IO connections.

  \ingroup things
  \inmodule libnymea

  Generic IO connections allow a user to connect states of two devices so they'll be synced whenever
  those states change.

  \sa ThingManager::connectIO

*/

#include "ioconnection.h"

IOConnection::IOConnection()
{

}

/*! Constructs a new IOConnection object. */
IOConnection::IOConnection(const IOConnectionId &id, const ThingId &inputThing, const StateTypeId &inputState, const ThingId &outputThing, const StateTypeId &outputState, bool inverted):
    m_id(id),
    m_inputThingId(inputThing),
    m_inputStateTypeId(inputState),
    m_outputThingId(outputThing),
    m_outputStateTypeId(outputState),
    m_inverted(inverted)
{

}

/*! Returns the ID of this connection object. */
IOConnectionId IOConnection::id() const
{
    return m_id;
}

/*! Returns the ID of the input thing for this connection. */
ThingId IOConnection::inputThingId() const
{
    return m_inputThingId;
}

/*! Returns the input state type ID for this connection. */
StateTypeId IOConnection::inputStateTypeId() const
{
    return m_inputStateTypeId;
}

/*! Returns the ID of the output thing for this connection. */
ThingId IOConnection::outputThingId() const
{
    return m_outputThingId;
}

/*! Returns the output state type  ID for this connection. */
StateTypeId IOConnection::outputStateTypeId() const
{
    return m_outputStateTypeId;
}

/*! Returns whether the connection is inverted or not. */
bool IOConnection::inverted() const
{
    return m_inverted;
}

QVariant IOConnections::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void IOConnections::put(const QVariant &variant)
{
    append(variant.value<IOConnection>());
}
