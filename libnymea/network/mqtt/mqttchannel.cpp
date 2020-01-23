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
    \class MqttChannel
    \brief Represents a isolated channel between a device and aplugin on the nymea internal MQTT broker

    \ingroup hardware
    \inmodule libnymea

    The MQTT channel class holds the required data to connect to the nymea internal MQTT broker.
*/

/*! \fn QString MqttChannel::clientId();
    Returns the clientId used to connect to the broker.

    \sa username() \sa password()
*/

/*! \fn QString MqttChannel::username();
    Returns the username used to connect to the broker.

    \sa clientId() \sa password()
*/

/*! \fn QString MqttChannel::password();
    Returns the password used to connect to the broker.

    \sa username() \sa clientId()
*/

/*! \fn QString MqttChannel::serverAddress();
    Returns the server address for the client to connect to.

    \sa username() \sa password()
*/

/*! \fn QString MqttChannel::serverPort();
    Returns the server port used to connect to the broker.

    \sa username() \sa password()
*/

/*! \fn QString MqttChannel::topicPrefix();
    Returns the topic prefix allowed in the broker's policy for this channel.
    Clients connecting to this channel are allowed to subscribe to "topicPrefix/#" as well
    as publishing to "topicPrefix/..."
*/

#include "mqttchannel.h"


MqttChannel::MqttChannel(QObject *parent): QObject(parent)
{

}

MqttChannel::~MqttChannel()
{

}
