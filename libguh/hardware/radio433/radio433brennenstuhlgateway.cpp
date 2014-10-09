/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "radio433brennenstuhlgateway.h"

Radio433BrennenstuhlGateway::Radio433BrennenstuhlGateway(QObject *parent) :
    QObject(parent)
{
    m_gatewayDiscovery = new QUdpSocket(this);
    m_gateway = new QUdpSocket(this);

    // connect(m_gateway, &QAbstractSocket::error, this, &Radio433BrennenstuhlGateway::gatewayError) -> does not work...runtime connection error.
    connect(m_gateway, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(gatewayError(QAbstractSocket::SocketError)));

    m_available = false;

    // TODO: maby enable or disable hardware search if the usern knows that he has not this gateway...
    // TODO: auto rediscover if lost connection
}

bool Radio433BrennenstuhlGateway::sendData(int delay, QList<int> rawData)
{
    if(m_gateway){
        QByteArray data;
        QByteArray message;

        // bring rawData list to a single ByteArray, values separated by ','
        foreach (int value, rawData) {
            data.append(QString::number(value) + ",");
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

        message.append("TXP:0,0,10,0," + QString::number(delay) + "," + QString::number(rawData.count()/2) + "," + data + ";");

        if(m_gateway->writeDatagram(message,m_gatewayAddress,m_gatewayPort) > 0){
            m_available = true;
            return true;
        }
    }
    return false;
}

bool Radio433BrennenstuhlGateway::enable()
{
    if(!m_gatewayDiscovery->bind(49880, QUdpSocket::ShareAddress)){
        qWarning() << "ERROR: Radio 433 MHz Brennenstuhl LAN Gateway discovery could not bind to port 49880";
        return false;
    }
    connect(m_gatewayDiscovery, &QUdpSocket::readyRead, this, &Radio433BrennenstuhlGateway::readDataDiscovery);

    discover();
    return true;
}

bool Radio433BrennenstuhlGateway::disable()
{
    m_gateway->close();
    m_gatewayDiscovery->close();
    return true;
}

void Radio433BrennenstuhlGateway::discover()
{
    // send search string to broadcast over port 49880
    m_gatewayDiscovery->writeDatagram("SEARCH HCGW", QHostAddress::Broadcast, 49880);
}

bool Radio433BrennenstuhlGateway::available()
{
    return m_available;
}

void Radio433BrennenstuhlGateway::gatewayDiscovered(QHostAddress address, int port)
{
    m_gatewayAddress = address;
    m_gatewayPort = port;

    if(!m_gateway->bind(m_gatewayPort,QUdpSocket::ShareAddress)){
        qWarning() << "ERROR: Radio 433 MHz Brennenstuhl LAN Gateway could not bind to port " << m_gatewayPort;
        m_available = false;
        emit availableChanged();
        return;
    }

    m_available = true;
    emit availableChanged();

    connect(m_gateway, &QUdpSocket::readyRead, this, &Radio433BrennenstuhlGateway::readDataGateway);
}

void Radio433BrennenstuhlGateway::readDataDiscovery()
{
    QByteArray data;
    QHostAddress address;
    quint16 port;

    // read the answere from the
    while (m_gatewayDiscovery->hasPendingDatagrams()) {
        data.resize(m_gatewayDiscovery->pendingDatagramSize());
        m_gatewayDiscovery->readDatagram(data.data(), data.size(), &address, &port);
    }

    if(data.startsWith("HCGW:")){
        // qDebug() << "found Brennenstuhl LAN gateway: " << address.toString() << port;
        gatewayDiscovered(address,port);
    }
}

void Radio433BrennenstuhlGateway::readDataGateway()
{
    QByteArray data;
    QHostAddress sender;
    quint16 udpPort;

    // read the answere from the gateway
    while (m_gateway->hasPendingDatagrams()) {
        data.resize(m_gateway->pendingDatagramSize());
        m_gateway->readDatagram(data.data(), data.size(), &sender, &udpPort);
    }
}

void Radio433BrennenstuhlGateway::gatewayError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QUdpSocket *gateway = static_cast<QUdpSocket*>(sender());
    qDebug() << "--> ERROR: Radio 433 MHz Brennenstuhl LAN Gateway disabled: " << gateway->errorString();
    m_available = false;
}
