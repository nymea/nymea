/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "tveventhandler.h"

TvEventHandler::TvEventHandler(QObject *parent, QHostAddress host, int port) :
    QTcpServer(parent),m_host(host),m_port(port)
{
    listen(QHostAddress::AnyIPv4,m_port);
    m_disabled = false;
    m_expectingData = false;

    //TODO: handle ip address change (dhcp) notification from the tv!
}

void TvEventHandler::incomingConnection(qintptr socket)
{
    if(m_disabled){
        return;
    }

    QTcpSocket* tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &TvEventHandler::readClient);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &TvEventHandler::discardClient);
    tcpSocket->setSocketDescriptor(socket);

    //qDebug() << "incomming connection" << s->peerAddress().toString() << s->peerName();
}

void TvEventHandler::readClient()
{
    if(m_disabled){
        return;
    }

    QTcpSocket* socket = (QTcpSocket*)sender();

    // reject everything, except the tv
    if(socket->peerAddress() != m_host){
        qWarning() << "reject connection from " << socket->peerAddress().toString();
        socket->close();
        if (socket->state() == QTcpSocket::UnconnectedState) {
            delete socket;
        }
        return;
    }


    // the tv sends first the header (POST /udap/api/.... HTTP/1.1)
    // in the scond package the tv sends the information (xml format)
    while(!socket->atEnd()){
        QByteArray data = socket->readAll();

        // check if we got information
        if(data.startsWith("<?xml") && m_expectingData){
            m_expectingData = false;

            // Answere with OK
            QTextStream textStream(socket);
            textStream.setAutoDetectUnicode(true);
            textStream << "HTTP/1.0 200 OK\r\n"
                          "Content-Type: text/html; charset=\"utf-8\"\r\n"
                          "User-Agent: UDAP/2.0 guh\r\n"
                       << "Date: " << QDateTime::currentDateTime().toString() << "\n";

            emit eventOccured(data);
        }

        // check if we got header
        if (data.startsWith("POST") && !m_expectingData) {
            m_expectingData = true;
//            QStringList tokens = QString(data).split(QRegExp("[ \r\n][ \r\n]*"));
//            qDebug() << "==================================";
//            qDebug() << "event occured" << "http://" << m_host.toString() << ":" << m_port << tokens[1];
        }
    }
}

void TvEventHandler::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

void TvEventHandler::enable()
{
    m_disabled = false;
}

void TvEventHandler::disable()
{
    m_disabled = true;
}
