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

#ifndef RADIO433BRENNENSTUHLGATEWAY_H
#define RADIO433BRENNENSTUHLGATEWAY_H

#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

#include <libnymea.h>

namespace nymeaserver {

class Radio433BrennenstuhlGateway : public QObject
{
    Q_OBJECT
public:
    explicit Radio433BrennenstuhlGateway(QObject *parent = nullptr);

    bool sendData(int delay, QList<int> rawData, int repetitions);
    bool enable();
    bool disable();
    bool available();

private:
    bool m_available;
    QUdpSocket *m_gateway;
    QHostAddress m_gatewayAddress;
    int m_port;

    QTimer *m_discoverTimer;
    QTimer *m_timeout;

    void discover();

signals:
    void availableChanged(const bool &available);

private slots:
    void readData();
    void gatewayError(QAbstractSocket::SocketError error);
    void timeout();
};

} // namespace nymeaserver

#endif // RADIO433BRENNENSTUHLGATEWAY_H
