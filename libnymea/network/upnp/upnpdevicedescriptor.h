// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef UPNPDEVICEDESCRIPTOR_H
#define UPNPDEVICEDESCRIPTOR_H

#include <QDebug>
#include <QHostAddress>
#include <QUrl>

#include "libnymea.h"

// reference: http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf

class LIBNYMEA_EXPORT UpnpDeviceDescriptor
{
public:
    explicit UpnpDeviceDescriptor();

    void setLocation(const QUrl &location);
    QUrl location() const;

    void setHostAddress(const QHostAddress &hostAddress);
    QHostAddress hostAddress() const;

    void setPort(const int &port);
    int port() const;

    void setDeviceType(const QString &deviceType);
    QString deviceType() const;

    void setFriendlyName(const QString &friendlyName);
    QString friendlyName() const;

    void setManufacturer(const QString &manufacturer);
    QString manufacturer() const;

    void setManufacturerURL(const QUrl &manufacturerURL);
    QUrl manufacturerURL() const;

    void setModelDescription(const QString &modelDescription);
    QString modelDescription() const;

    void setModelName(const QString &modelName);
    QString modelName() const;

    void setModelNumber(const QString &modelNumber);
    QString modelNumber() const;

    void setModelURL(const QUrl &modelURL);
    QUrl modelURL() const;

    void setSerialNumber(const QString &serialNumber);
    QString serialNumber() const;

    void setUuid(const QString &uuid);
    QString uuid() const;

    void setUpc(const QString &upc);
    QString upc() const;


private:
    QUrl m_location;
    QHostAddress m_hostAddress;
    int m_port;
    QString m_deviceType;
    QString m_friendlyName;
    QString m_manufacturer;
    QUrl m_manufacturerURL;
    QString m_modelDescription;
    QString m_modelName;
    QString m_modelNumber;
    QUrl m_modelURL;
    QString m_serialNumber;
    QString m_uuid;
    QString m_upc;
};

Q_DECLARE_METATYPE(UpnpDeviceDescriptor)
QDebug operator<< (QDebug debug, const UpnpDeviceDescriptor &upnpDeviceDescriptor);

#endif // UPNPDEVICEDESCRIPTOR_H
