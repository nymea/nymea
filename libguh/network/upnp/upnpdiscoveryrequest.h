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

#ifndef UPNPDISCOVERYREQUEST_H
#define UPNPDISCOVERYREQUEST_H

#include <QObject>
#include <QDebug>

#include "upnpdiscovery.h"
#include "upnpdevicedescriptor.h"
#include "libguh.h"
#include "typeutils.h"

class UpnpDiscovery;

class LIBGUH_EXPORT UpnpDiscoveryRequest : public QObject
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
