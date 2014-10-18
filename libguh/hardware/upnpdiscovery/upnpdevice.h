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

#ifndef UPNPDEVICE_H
#define UPNPDEVICE_H

#include <QObject>

#include "upnpdevicedescriptor.h"

class UpnpDevice : public QObject
{
    Q_OBJECT
public:
    explicit UpnpDevice(QObject *parent = 0, UpnpDeviceDescriptor upnpDeviceDescriptor = UpnpDeviceDescriptor());

    QUrl location();
    QHostAddress hostAddress() const;
    int port() const;
    QString deviceType() const;
    QString friendlyName() const;
    QString manufacturer() const;
    QUrl manufacturerURL() const;
    QString modelDescription() const;
    QString modelName() const;
    QString modelNumber() const;
    QUrl modelURL() const;
    QString serialNumber() const;
    QString uuid() const;
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

signals:

public slots:

};

#endif // UPNPDEVICE_H
