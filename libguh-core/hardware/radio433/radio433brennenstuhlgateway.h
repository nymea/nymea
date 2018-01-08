/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RADIO433BRENNENSTUHLGATEWAY_H
#define RADIO433BRENNENSTUHLGATEWAY_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>

#include "libguh.h"

namespace guhserver {

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

}

#endif // RADIO433BRENNENSTUHLGATEWAY_H
