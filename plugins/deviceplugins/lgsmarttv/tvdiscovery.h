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

#ifndef TVDISCOVERY_H
#define TVDISCOVERY_H

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

#include "tvdevice.h"

class TvDiscovery : public QUdpSocket
{
    Q_OBJECT
public:
    explicit TvDiscovery(QObject *parent = 0);

private:
    QHostAddress m_host;
    qint16 m_port;

    QTimer *m_timeout;
    QList<TvDevice*> m_tvList;

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_deviceInformationReplay;

    QByteArray m_deviceInformationData;
    bool checkXmlData(QByteArray data);
    QString printXmlData(QByteArray data);

signals:
    void discoveryDone(const QList<TvDevice*> deviceList);

private slots:
    void error(QAbstractSocket::SocketError error);
    void readData();
    void discoverTimeout();

    void requestDeviceInformation(TvDevice *device);
    void replyFinished(QNetworkReply *reply);
    void parseDeviceInformation(QByteArray data);

public slots:
    void discover(int timeout);

};

#endif // TVDISCOVERY_H
