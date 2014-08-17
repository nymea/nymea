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

#ifndef TVEVENTHANDLER_H
#define TVEVENTHANDLER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class TvEventHandler : public QTcpServer
{
    Q_OBJECT
public:
    explicit TvEventHandler(QObject *parent = 0, QHostAddress host = QHostAddress(), int port = 8080);
    void incomingConnection(qintptr socket) override;

private:
    QHostAddress m_host;
    int m_port;
    bool m_disabled;

    bool m_expectingData;

    QNetworkAccessManager *m_manager;

signals:
    void eventOccured(const QByteArray &path);
    void byebyeEvent();

private slots:
    void readClient();
    void discardClient();

public slots:
    void enable();
    void disable();

};

#endif // TVEVENTHANDLER_H
