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

#ifndef WEMODISCOVERY_H
#define WEMODISCOVERY_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

#include "wemoswitch.h"

class WemoDiscovery : public QUdpSocket
{
    Q_OBJECT
public:
    explicit WemoDiscovery(QObject *parent = 0);

private:
    QHostAddress m_host;
    qint16 m_port;

    QTimer *m_timeout;

    QNetworkAccessManager *m_manager;

    QByteArray m_deviceInformationData;
    bool checkXmlData(QByteArray data);
    QString printXmlData(QByteArray data);

    QList<WemoSwitch*> m_deviceList;

signals:
    void discoveryDone(QList<WemoSwitch*> deviceList);

private slots:
    void error(QAbstractSocket::SocketError error);
    void sendDiscoverMessage();
    void readData();
    void discoverTimeout();

    void requestDeviceInformation(QUrl location);
    void replyFinished(QNetworkReply *reply);
    void parseDeviceInformation(QByteArray data);

public slots:
    void discover(int timeout);


};

#endif // WEMODISCOVERY_H
