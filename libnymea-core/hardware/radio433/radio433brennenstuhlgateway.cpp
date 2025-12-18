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

#include "radio433brennenstuhlgateway.h"
#include "loggingcategories.h"

namespace nymeaserver {

Radio433BrennenstuhlGateway::Radio433BrennenstuhlGateway(QObject *parent)
    : QObject(parent)
{
    m_port = 49880;

    // UDP socket to sending data to gateway
    m_gateway = new QUdpSocket(this);
    connect(m_gateway, &QUdpSocket::readyRead, this, &Radio433BrennenstuhlGateway::readData);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_gateway, &QUdpSocket::errorOccurred, this, &Radio433BrennenstuhlGateway::gatewayError);
#else
    connect(m_gateway, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(gatewayError(QAbstractSocket::SocketError)));
#endif

    // Timer for discovery of the Gateway
    m_discoverTimer = new QTimer(this);
    m_discoverTimer->setInterval(5000);
    connect(m_discoverTimer, &QTimer::timeout, this, &Radio433BrennenstuhlGateway::discover);

    // Timer to detect discovery timeout
    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);
    m_timeout->setInterval(3000);
    connect(m_timeout, &QTimer::timeout, this, &Radio433BrennenstuhlGateway::timeout);
}

bool Radio433BrennenstuhlGateway::sendData(int delay, QList<int> rawData, int repetitions)
{
    QByteArray data;
    QByteArray message;

    // bring rawData list to a single ByteArray, values separated by ','
    foreach (int value, rawData) {
        data.append(QString("%1,").arg(value).toUtf8());
    }

    /* Protocol:
     *  Elro Example
     * "TXP:0,0,10,0,350,25,  1,31,1,3,3,1,1,3,3,1,1,3,3,1,1,3,3,1,1,3,3,1,1,3,3,1,1,3,1,3,1,3,3,1,1,3,3,1,1,3,3,1,1,3,1,3,1,3,3,1, ;"
     *
     * TXP:     |   send command
     * 0,0      |   unknown
     * 10,      |   repeatings of command
     * 0        |   pause between 2 commands [us]
     * 350      |   delayLength [us]
     * 25       |   number of bits to send (2 sync timings + 48 data timings = 50 timongis ..... 1 bit = 2 timings -> 25 bit)
     * 1,3,3,...|   pulse data starting with HIGH (1* delayLength HIGH, 3 * delayLength LOW ....)
     * ;        |   end of command
     */

    message.append(QString("TXP:0,0,%1,0,%2,%3,").arg(repetitions).arg(delay).arg(rawData.count() / 2).toUtf8() + data + QByteArray(";"));

    if (m_gateway->writeDatagram(message, m_gatewayAddress, m_port) > 0) {
        m_available = true;
    } else {
        qCWarning(dcHardware) << "could not send command to Brennenstihl Gateway";
        m_available = false;
        emit availableChanged(false);
        return false;
    }
    return true;
}

bool Radio433BrennenstuhlGateway::enable()
{
    m_available = false;

    if (!m_gateway->bind(m_port, QUdpSocket::ShareAddress)) {
        qCWarning(dcHardware) << "Radio 433 MHz Brennenstuhl LAN Gateway discovery could not bind to port " << m_port;
        return false;
    }

    discover();
    m_discoverTimer->start();
    return true;
}

bool Radio433BrennenstuhlGateway::disable()
{
    m_discoverTimer->stop();
    m_gateway->close();
    return true;
}

void Radio433BrennenstuhlGateway::discover()
{
    // send search string to broadcast over port 49880
    m_gateway->writeDatagram("SEARCH HCGW", QHostAddress::Broadcast, m_port);
    m_timeout->start();
}

bool Radio433BrennenstuhlGateway::available()
{
    return m_available;
}

void Radio433BrennenstuhlGateway::readData()
{
    QByteArray data;
    QHostAddress address;
    quint16 port;

    // read the answere from the
    while (m_gateway->hasPendingDatagrams()) {
        data.resize(m_gateway->pendingDatagramSize());
        m_gateway->readDatagram(data.data(), data.size(), &address, &port);
    }

    if (data.startsWith("HCGW:")) {
        m_timeout->stop();
        if (!m_available) {
            m_gatewayAddress = address;
            m_available = true;
            emit availableChanged(true);
        }
    }
}

void Radio433BrennenstuhlGateway::gatewayError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QUdpSocket *gateway = static_cast<QUdpSocket *>(sender());
    if (m_available) {
        qCWarning(dcHardware) << "--> ERROR: Radio 433 MHz Brennenstuhl LAN Gateway socket error: " << gateway->errorString();
        m_available = false;
        emit availableChanged(false);
    }
}

void Radio433BrennenstuhlGateway::timeout()
{
    if (m_available) {
        m_available = false;
        emit availableChanged(false);
    }
}

} // namespace nymeaserver
