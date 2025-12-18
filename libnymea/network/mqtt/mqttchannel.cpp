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

MqttChannel::MqttChannel(QObject *parent)
    : QObject(parent)
{}

MqttChannel::~MqttChannel() {}
