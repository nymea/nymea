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

#include "upnpdevicedescriptor.h"

UpnpDeviceDescriptor::UpnpDeviceDescriptor()
{
}

void UpnpDeviceDescriptor::setLocation(const QUrl &location)
{
    m_location = location;
}

QUrl UpnpDeviceDescriptor::location() const
{
    return m_location;
}

void UpnpDeviceDescriptor::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

QHostAddress UpnpDeviceDescriptor::hostAddress() const
{
    return m_hostAddress;
}

void UpnpDeviceDescriptor::setPort(const int &port)
{
    m_port = port;
}

int UpnpDeviceDescriptor::port() const
{
    return m_port;
}

void UpnpDeviceDescriptor::setDeviceType(const QString &deviceType)
{
    m_deviceType = deviceType;
}

QString UpnpDeviceDescriptor::deviceType() const
{
    return m_deviceType;
}

void UpnpDeviceDescriptor::setFriendlyName(const QString &friendlyName)
{
    m_friendlyName = friendlyName;
}

QString UpnpDeviceDescriptor::friendlyName() const
{
    return m_friendlyName;
}

void UpnpDeviceDescriptor::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

QString UpnpDeviceDescriptor::manufacturer() const
{
    return m_manufacturer;
}

void UpnpDeviceDescriptor::setManufacturerURL(const QUrl &manufacturerURL)
{
    m_manufacturerURL = manufacturerURL;
}

QUrl UpnpDeviceDescriptor::manufacturerURL() const
{
    return m_manufacturerURL;
}

void UpnpDeviceDescriptor::setModelDescription(const QString &modelDescription)
{
    m_modelDescription = modelDescription;
}

QString UpnpDeviceDescriptor::modelDescription() const
{
    return m_modelDescription;
}

void UpnpDeviceDescriptor::setModelName(const QString &modelName)
{
    m_modelName = modelName;
}

QString UpnpDeviceDescriptor::modelName() const
{
    return m_modelName;
}

void UpnpDeviceDescriptor::setModelNumber(const QString &modelNumber)
{
    m_modelNumber = modelNumber;
}

QString UpnpDeviceDescriptor::modelNumber() const
{
    return m_modelNumber;
}

void UpnpDeviceDescriptor::setModelURL(const QUrl &modelURL)
{
    m_modelURL = modelURL;
}

QUrl UpnpDeviceDescriptor::modelURL() const
{
    return m_modelURL;
}

void UpnpDeviceDescriptor::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

QString UpnpDeviceDescriptor::serialNumber() const
{
    return m_serialNumber;
}

void UpnpDeviceDescriptor::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString UpnpDeviceDescriptor::uuid() const
{
    return m_uuid;
}

void UpnpDeviceDescriptor::setUpc(const QString &upc)
{
    m_upc = upc;
}

QString UpnpDeviceDescriptor::upc() const
{
    return m_upc;
}

QDebug operator<<(QDebug debug, const UpnpDeviceDescriptor &upnpDeviceDescriptor)
{
    debug << "----------------------------------------------\n";
    debug << "UPnP device on " << upnpDeviceDescriptor.hostAddress().toString() << upnpDeviceDescriptor.port() << "\n";
    debug << "location              | " << upnpDeviceDescriptor.location() << "\n";
    debug << "friendly name         | " << upnpDeviceDescriptor.friendlyName() << "\n";
    debug << "manufacturer          | " << upnpDeviceDescriptor.manufacturer() << "\n";
    debug << "manufacturer URL      | " << upnpDeviceDescriptor.manufacturerURL().toString() << "\n";
    debug << "device type           | " << upnpDeviceDescriptor.deviceType() << "\n";
    debug << "model name            | " << upnpDeviceDescriptor.modelName() << "\n";
    debug << "model number          | " << upnpDeviceDescriptor.modelNumber() << "\n";
    debug << "model description     | " << upnpDeviceDescriptor.modelDescription() << "\n";
    debug << "model URL             | " << upnpDeviceDescriptor.modelURL().toString() << "\n";
    debug << "serial number         | " << upnpDeviceDescriptor.serialNumber() << "\n";
    debug << "UUID                  | " << upnpDeviceDescriptor.uuid() << "\n";
    debug << "UPC                   | " << upnpDeviceDescriptor.upc() << "\n\n";

    return debug;
}
