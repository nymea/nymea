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
IOConnection::IOConnection(const IOConnectionId &id, const ThingId &inputThing, const QString &inputState, const ThingId &outputThing, const QString &outputState, bool inverted):
    m_id(id),
    m_inputThingId(inputThing),
    m_inputState(inputState),
    m_outputThingId(outputThing),
    m_outputState(outputState),
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

void IOConnection::setInputStateTypeId(const StateTypeId &stateTypeId)
{
    m_inputStateTypeId = stateTypeId;
}

QString IOConnection::inputState() const
{
    return m_inputState;
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

void IOConnection::setOutputStateTypeId(const StateTypeId &outputStateTypeId)
{
    m_outputStateTypeId = outputStateTypeId;
}

QString IOConnection::outputState() const
{
    return m_outputState;
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
