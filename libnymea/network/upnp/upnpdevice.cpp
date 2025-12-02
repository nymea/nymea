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

/*!
    \class UpnpDevice
    \brief Describes an UPnP device.

    \ingroup types
    \inmodule libnymea

    This class represents a UPnP device with all parameters described in following documentation: \l{http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf}.

*/

#include "upnpdevice.h"

/*! Constructs a UpnpDevice with the given \a parent and the given \a upnpDeviceDescriptor
 \sa UpnpDeviceDescriptor,*/
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

/*! Returns the location URL of this UPnP device. */
QUrl UpnpDevice::location()
{
    return m_location;
}

/*! Sets the \a location URL of this UPnP device. */
void UpnpDevice::setLocation(const QUrl &location)
{
    m_location = location;
}

/*! Returns the host address of this UPnP device. */
QHostAddress UpnpDevice::hostAddress() const
{
    return m_hostAddress;
}

/*! Sets the \a hostAddress of this UPnP device. */
void UpnpDevice::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

/*! Returns the port of this UPnP device. */
int UpnpDevice::port() const
{
    return m_port;
}

/*! Sets the \a port of this UPnP device. */
void UpnpDevice::setPort(const int &port)
{
    m_port = port;
}

/*! Returns the type of this UPnP device. */
QString UpnpDevice::deviceType() const
{
    return m_deviceType;
}

/*! Sets the \a deviceType of this UPnP device. */
void UpnpDevice::setDeviceType(const QString &deviceType)
{
    m_deviceType = deviceType;
}

/*! Returns the friendly name of this UPnP device. */
QString UpnpDevice::friendlyName() const
{
    return m_friendlyName;
}

/*! Sets the \a friendlyName of this UPnP device. */
void UpnpDevice::setFriendlyName(const QString &friendlyName)
{
    m_friendlyName = friendlyName;
}

/*! Returns the manufacturer of this UPnP device. */
QString UpnpDevice::manufacturer() const
{
    return m_manufacturer;
}

/*! Sets the \a manufacturer of this UPnP device. */
void UpnpDevice::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

/*! Returns the manufacturer URL of this UPnP device. */
QUrl UpnpDevice::manufacturerURL() const
{
    return m_manufacturerURL;
}

/*! Sets the \a manufacturerURL of this UPnP device. */
void UpnpDevice::setManufacturerURL(const QUrl &manufacturerURL)
{
    m_manufacturerURL = manufacturerURL;
}

/*! Returns the model description of this UPnP device. */
QString UpnpDevice::modelDescription() const
{
    return m_modelDescription;
}

/*! Sets the \a modelDescription of this UPnP device. */
void UpnpDevice::setModelDescription(const QString &modelDescription)
{
    m_modelDescription = modelDescription;
}

/*! Returns the model name of this UPnP device. */
QString UpnpDevice::modelName() const
{
    return m_modelName;
}

/*! Sets the \a modelName of this UPnP device. */
void UpnpDevice::setModelName(const QString &modelName)
{
    m_modelName = modelName;
}

/*! Returns the model number of this UPnP device. */
QString UpnpDevice::modelNumber() const
{
    return m_modelNumber;
}

/*! Sets the \a modelNumber of this UPnP device. */
void UpnpDevice::setModelNumber(const QString &modelNumber)
{
    m_modelNumber = modelNumber;
}

/*! Returns the model URL of this UPnP device. */
QUrl UpnpDevice::modelURL() const
{
    return m_modelURL;
}

/*! Sets the \a modelURL of this UPnP device. */
void UpnpDevice::setModelURL(const QUrl &modelURL)
{
    m_modelURL = modelURL;
}

/*! Returns the serial number of this UPnP device. */
QString UpnpDevice::serialNumber() const
{
    return m_serialNumber;
}

/*! Sets the \a serialNumber of this UPnP device. */
void UpnpDevice::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

/*! Returns the uuid of this UPnP device. */
QString UpnpDevice::uuid() const
{
    return m_uuid;
}

/*! Sets the \a uuid of this UPnP device. */
void UpnpDevice::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

/*! Returns the UPC (Universal Product Code) of this UPnP device. */
QString UpnpDevice::upc() const
{
    return m_upc;
}

/*! Sets the \a upc (Universal Product Code) of this UPnP device. */
void UpnpDevice::setUpc(const QString &upc)
{
    m_upc = upc;
}
