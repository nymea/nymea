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

#ifndef MQTTPROVIDER_H
#define MQTTPROVIDER_H

#include <QObject>
#include <QHostAddress>

#include "typeutils.h"
#include "hardwareresource.h"
#include <mqttclient.h>

class MqttChannel;

class MqttProvider : public HardwareResource
{
    Q_OBJECT
public:
    explicit MqttProvider(QObject *parent = nullptr);

    virtual MqttChannel* createChannel(const QHostAddress &clientAddress, const QStringList &topicPrefixList = QStringList()) = 0;
    virtual MqttChannel* createChannel(const QString &clientId, const QHostAddress &clientAddress, const QStringList &topicPrefixList = QStringList()) = 0;
    virtual MqttChannel* createChannel(const QString &clientId, const QString &username, const QString &password, const QHostAddress &clientAddress, const QStringList &topicPrefixList = QStringList()) = 0;
    virtual void releaseChannel(MqttChannel *channel) = 0;

    virtual MqttClient* createInternalClient(const QString &clientId) = 0;
};

#endif // MQTTPROVIDER_H
