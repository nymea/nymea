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

QHostAddress UpnpDevice::hostAddress() const
{
    return m_hostAddress;
}

int UpnpDevice::port() const
{
    return m_port;
}

QString UpnpDevice::deviceType() const
{
    return m_deviceType;
}

QString UpnpDevice::friendlyName() const
{
    return m_friendlyName;
}

QString UpnpDevice::manufacturer() const
{
    return m_manufacturer;
}

QUrl UpnpDevice::manufacturerURL() const
{
    return m_manufacturerURL;
}

QString UpnpDevice::modelDescription() const
{
    return m_modelDescription;
}

QString UpnpDevice::modelName() const
{
    return m_modelName;
}

QString UpnpDevice::modelNumber() const
{
    return m_modelNumber;
}

QUrl UpnpDevice::modelURL() const
{
    return m_modelURL;
}

QString UpnpDevice::serialNumber() const
{
    return m_serialNumber;
}

QString UpnpDevice::uuid() const
{
    return m_uuid;
}

QString UpnpDevice::upc() const
{
    return m_upc;
}
