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

#ifndef MQTTCHANNEL_H
#define MQTTCHANNEL_H

#include <QObject>
#include <QHostAddress>

#include "libnymea.h"

class LIBNYMEA_EXPORT MqttChannel: public QObject
{
    Q_OBJECT
public:
    MqttChannel(QObject *parent = nullptr);
    virtual ~MqttChannel();

    virtual QString clientId() const = 0;
    virtual QString username() const = 0;
    virtual QString password() const = 0;
    virtual QHostAddress serverAddress() const = 0;
    virtual quint16 serverPort() const = 0;
    virtual QStringList topicPrefixList() const = 0;

    virtual void publish(const QString &topic, const QByteArray &payload) = 0;

    virtual bool isConnected() const = 0;

signals:
    void clientConnected(MqttChannel* channel);
    void clientDisconnected(MqttChannel* channel);
    void publishReceived(MqttChannel* channel, const QString &topic, const QByteArray &payload);
};

#endif // MQTTCHANNEL_H
