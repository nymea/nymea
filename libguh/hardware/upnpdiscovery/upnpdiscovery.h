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

#ifndef UPNPDISCOVERY_H
#define UPNPDISCOVERY_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "upnpdevicedescriptor.h"
#include "devicemanager.h"

// reference: http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf

class UpnpDiscovery : public QUdpSocket
{
    Q_OBJECT
public:
    explicit UpnpDiscovery(QObject *parent = 0);
    bool discoverDevices(const QString &searchTarget = "ssdp:all", const QString &userAgent = "", const PluginId &pluginId = PluginId());

private:
    QHostAddress m_host;
    qint16 m_port;
    QTimer *m_timer;
    PluginId m_pluginId;
    QString m_searchTarget;
    QString m_userAgent;

    QNetworkAccessManager *m_networkAccessManager;
    QHash<QNetworkReply*,UpnpDeviceDescriptor> m_informationRequestList;
    QList<UpnpDeviceDescriptor> m_deviceList;

    void requestDeviceInformation(const UpnpDeviceDescriptor &upnpDeviceDescriptor);

signals:
    void discoveryFinished(const QList<UpnpDeviceDescriptor> &deviceDescriptorList, const PluginId & pluginId);
    void upnpNotify(const QByteArray &notifyMessage);

private slots:
    void error(QAbstractSocket::SocketError error);
    void readData();
    void replyFinished(QNetworkReply *reply);
    void discoverTimeout();

public slots:
};

#endif // UPNPDISCOVERY_H
