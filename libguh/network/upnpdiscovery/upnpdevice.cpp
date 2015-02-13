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

#include "upnpdevice.h"

UpnpDevice::UpnpDevice(QObject *parent, UpnpDeviceDescriptor upnpDeviceDescriptor) :
    QObject(parent)
{
    m_location = upnpDeviceDescriptor.location();
    m_hostAddress = upnpDeviceDescriptor.hostAddress();
    m_port = upnpDeviceDescriptor.port();
    m_deviceType = upnpDeviceDescriptor.deviceType();
    m_friendlyName = upnpDeviceDescriptor.friendlyName();
    m_manufacturer = upnpDeviceDescriptor.manufacturer();
    m_manufacturerURL = upnpDeviceDescriptor.manufacturerURL();
    m_modelDescription = upnpDeviceDescriptor.modelDescription();
    m_modelName = upnpDeviceDescriptor.modelName();
    m_modelNumber = upnpDeviceDescriptor.modelNumber();
    m_modelURL = upnpDeviceDescriptor.modelURL();
    m_serialNumber = upnpDeviceDescriptor.serialNumber();
    m_uuid = upnpDeviceDescriptor.uuid();
    m_upc = upnpDeviceDescriptor.upc();
}

QUrl UpnpDevice::location()
{
    return m_location;
}

void UpnpDevice::setLocation(const QUrl &location)
{
    m_location = location;
}

QHostAddress UpnpDevice::hostAddress() const
{
    return m_hostAddress;
}

void UpnpDevice::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

int UpnpDevice::port() const
{
    return m_port;
}

void UpnpDevice::setPort(const int &port)
{
    m_port = port;
}

QString UpnpDevice::deviceType() const
{
    return m_deviceType;
}

void UpnpDevice::setDeviceType(const QString &deviceType)
{
    m_deviceType = deviceType;
}

QString UpnpDevice::friendlyName() const
{
    return m_friendlyName;
}

void UpnpDevice::setFriendlyName(const QString &friendlyName)
{
    m_friendlyName = friendlyName;
}

QString UpnpDevice::manufacturer() const
{
    return m_manufacturer;
}

void UpnpDevice::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

QUrl UpnpDevice::manufacturerURL() const
{
    return m_manufacturerURL;
}

void UpnpDevice::setManufacturerURL(const QUrl &manufacturerURL)
{
    m_manufacturerURL = manufacturerURL;
}

QString UpnpDevice::modelDescription() const
{
    return m_modelDescription;
}

void UpnpDevice::setModelDescription(const QString &modelDescription)
{
    m_modelDescription = modelDescription;
}

QString UpnpDevice::modelName() const
{
    return m_modelName;
}

void UpnpDevice::setModelName(const QString &modelName)
{
    m_modelName = modelName;
}

QString UpnpDevice::modelNumber() const
{
    return m_modelNumber;
}

void UpnpDevice::setModelNumber(const QString &modelNumber)
{
    m_modelNumber = modelNumber;
}

QUrl UpnpDevice::modelURL() const
{
    return m_modelURL;
}

void UpnpDevice::setModelURL(const QUrl &modelURL)
{
    m_modelURL = modelURL;
}

QString UpnpDevice::serialNumber() const
{
    return m_serialNumber;
}

void UpnpDevice::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

QString UpnpDevice::uuid() const
{
    return m_uuid;
}

void UpnpDevice::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString UpnpDevice::upc() const
{
    return m_upc;
}

void UpnpDevice::setUpc(const QString &upc)
{
    m_upc = upc;
}
