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

#include "maxcubediscovery.h"
#include "extern-plugininfo.h"

MaxCubeDiscovery::MaxCubeDiscovery(QObject *parent) :
    QObject(parent)
{
    // UDP broadcast for cube detection in the network
    m_udpSocket = new QUdpSocket(this);
    m_port = 23272;
    m_udpSocket->bind(m_port,QUdpSocket::ShareAddress);

    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);

    connect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(m_timeout,SIGNAL(timeout()),this,SLOT(discoverTimeout()));
}

void MaxCubeDiscovery::detectCubes()
{
    m_cubeList.clear();

    // broadcast the hello message, every cube should respond with a 26 byte message
    m_udpSocket->writeDatagram("eQ3Max*.**********I", QHostAddress::Broadcast, m_port);
    m_timeout->start(1500);
}

void MaxCubeDiscovery::readData()
{
    QByteArray data;
    QHostAddress sender;
    quint16 udpPort;

    // read the answere from the
    while (m_udpSocket->hasPendingDatagrams()) {
        data.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(data.data(), data.size(), &sender, &udpPort);
    }
    if(!data.isEmpty() && data.contains("eQ3MaxAp")){

        QString serialNumber = data.mid(8,10);
        QByteArray rfAddress = data.mid(21,3).toHex();
        int firmware = data.mid(24,2).toHex().toInt();
        qint16 port;

        // set port depending on the firmware
        if(firmware < 109){
            port= 80;
        }else{
            port = 62910;
        }

        MaxCube *cube = new MaxCube(this, serialNumber, sender, port);
        cube->setRfAddress(rfAddress);
        m_cubeList.append(cube);
    }
}

void MaxCubeDiscovery::discoverTimeout()
{
    emit cubesDetected(m_cubeList);
}
