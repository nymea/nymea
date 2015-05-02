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

#include "lircdclient.h"

#include <QDebug>
#include <QLocalSocket>

LircClient::LircClient(QObject *parent) :
    QObject(parent)
{
    m_socket = new QLocalSocket(this);
    QObject::connect(m_socket, &QLocalSocket::readyRead, this, &LircClient::readyRead);
}

bool LircClient::connect()
{
    m_socket->connectToServer("/var/run/lirc/lircd", QIODevice::ReadWrite);
    if (!m_socket->isOpen()) {
        qWarning() << "--> Lirc daemon NOT available.";
        return false;
    }
    m_socket->write("LIST\n");
    qDebug() << "--> Lirc daemon available.";
    return true;
}

void LircClient::readyRead()
{
//    qDebug() << "got data" << m_socket->readAll();

    bool inBlock = false;
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        //qDebug() << "got line:" << line;
        if (line == "BEGIN") {
            inBlock = true;
            continue;
        }
        if (line == "LIST") {
            if (m_socket->readLine().trimmed() == "SUCCESS") {
                readRemotes();
            } else {
                qWarning() << "Error reading remotes from Lircd";
            }
            continue;
        }
        if ( line == "END") {
            inBlock = false;
            continue;
        }

        if (!inBlock) {
            QList<QByteArray> parts = line.split(' ');
            if (parts.count() != 4) {
                qWarning() << "Don't understand IR command. ignoring...";
                continue;
            }
            qDebug() << "emitting buttonpress";
            emit buttonPressed(QString(parts.at(3)), QString(parts.at(2)), parts.at(1).toInt());
        }
    }
}

void LircClient::readRemotes()
{
    m_socket->readLine(); // IGNORE DATA
    int remoteCount = m_socket->readLine().trimmed().toInt();
    qDebug() << "found" << remoteCount << "lirc remotes";
    for (int i = 0; i < remoteCount; i++) {
        QByteArray line = m_socket->readLine().trimmed();
        m_remotes.append(line);
    }
}

