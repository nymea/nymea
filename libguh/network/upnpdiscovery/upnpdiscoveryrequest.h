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

#ifndef UPNPDISCOVERYREQUEST_H
#define UPNPDISCOVERYREQUEST_H

#include <QObject>
#include <QDebug>
#include "upnpdiscovery.h"
#include "upnpdevicedescriptor.h"
#include "typeutils.h"

class UpnpDiscovery;

class UpnpDiscoveryRequest : public QObject
{
    Q_OBJECT
public:
    explicit UpnpDiscoveryRequest(UpnpDiscovery *upnpDiscovery, PluginId pluginId, QString searchTarget, QString userAgent);

    void discover();
    void addDeviceDescriptor(const UpnpDeviceDescriptor &deviceDescriptor);
    QNetworkRequest createNetworkRequest(UpnpDeviceDescriptor deviveDescriptor);
    QList<UpnpDeviceDescriptor> deviceList() const;

    PluginId pluginId() const;
    QString searchTarget() const;
    QString userAgent() const;

private:
    UpnpDiscovery *m_upnpDiscovery;
    QTimer *m_timer;
    PluginId m_pluginId;
    QString m_searchTarget;
    QString m_userAgent;

    QList<UpnpDeviceDescriptor> m_deviceList;

signals:
    void discoveryTimeout();

public slots:

};

#endif // UPNPDISCOVERYREQUEST_H
