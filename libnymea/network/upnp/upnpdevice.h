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

#ifndef UPNPDEVICE_H
#define UPNPDEVICE_H

#include <QObject>

#include "libnymea.h"
#include "upnpdevicedescriptor.h"

class LIBNYMEA_EXPORT UpnpDevice : public QObject
{
    Q_OBJECT
public:
    explicit UpnpDevice(QObject *parent = nullptr, UpnpDeviceDescriptor upnpDeviceDescriptor = UpnpDeviceDescriptor());

    QUrl location();
    void setLocation(const QUrl &location);

    QHostAddress hostAddress() const;
    void setHostAddress(const QHostAddress &hostAddress);

    int port() const;
    void setPort(const int &port);

    QString deviceType() const;
    void setDeviceType(const QString &deviceType);

    QString friendlyName() const;
    void setFriendlyName(const QString &friendlyName);

    QString manufacturer() const;
    void setManufacturer(const QString &manufacturer);

    QUrl manufacturerURL() const;
    void setManufacturerURL(const QUrl &manufacturerURL);

    QString modelDescription() const;
    void setModelDescription(const QString &modelDescription);

    QString modelName() const;
    void setModelName(const QString &modelName);

    QString modelNumber() const;
    void setModelNumber(const QString &modelNumber);

    QUrl modelURL() const;
    void setModelURL(const QUrl &modelURL);

    QString serialNumber() const;
    void setSerialNumber(const QString &serialNumber);

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString upc() const;
    void setUpc(const QString &upc);

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

signals:

public slots:
};

#endif // UPNPDEVICE_H
