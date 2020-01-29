/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class DeviceDescriptor
    \brief Holds the description of a \l{Device}.

    \ingroup devices
    \inmodule libnymea

    An DeviceDescriptor describes an \l{Device} in order to match it with a \l{nymeaserver::Rule}.

    \sa Device
*/


#include "devicedescriptor.h"


/*! Construct a DeviceDescriptor. */
DeviceDescriptor::DeviceDescriptor():
    m_id(DeviceDescriptorId::createDeviceDescriptorId())
{

}

/*! Construct a DeviceDescriptor with the given \a deviceClassId, \a title, \a description and \a parentDeviceId.*/
DeviceDescriptor::DeviceDescriptor(const DeviceClassId &deviceClassId, const QString &title, const QString &description, const DeviceId &parentDeviceId):
    m_id(DeviceDescriptorId::createDeviceDescriptorId()),
    m_deviceClassId(deviceClassId),
    m_title(title),
    m_description(description),
    m_parentDeviceId(parentDeviceId)
{

}

/*! Construct a DeviceDescriptor with the given \a id, \a deviceClassId, \a title, \a description and \a parentDeviceId.*/
DeviceDescriptor::DeviceDescriptor(const DeviceDescriptorId &id, const DeviceClassId &deviceClassId, const QString &title, const QString &description, const DeviceId &parentDeviceId) :
    m_id(id),
    m_deviceClassId(deviceClassId),
    m_title(title),
    m_description(description),
    m_parentDeviceId(parentDeviceId)
{
}

/*! Returns true, if this DeviceDescriptor is valid. A DeviceDescriptor is valid, if the id and the deviceClassId are set.
 *  \sa id(), deviceClassId(), */
bool DeviceDescriptor::isValid() const
{
    return !m_id.isNull() && !m_deviceClassId.isNull();
}

/*! Returns the id of this DeviceDescriptor. */
DeviceDescriptorId DeviceDescriptor::id() const
{
    return m_id;
}

/*! Returns the deviceClassId of this DeviceDescriptor. */
DeviceClassId DeviceDescriptor::deviceClassId() const
{
    return m_deviceClassId;
}

/*! Returns the \a deviceId of the device matching this descriptor. */
DeviceId DeviceDescriptor::deviceId() const
{
    return m_deviceId;
}

/*! Set the \a deviceId of the device matching this device descriptor. */
void DeviceDescriptor::setDeviceId(const DeviceId &deviceId)
{
    m_deviceId = deviceId;
}

/*! Returns the name of this DeviceDescriptor. */
QString DeviceDescriptor::title() const
{
    return m_title;
}

/*! Sets the \a title of this DeviceDescriptor. */
void DeviceDescriptor::setTitle(const QString &title)
{
    m_title = title;
}

/*! Returns the description of this DeviceDescriptor. */
QString DeviceDescriptor::description() const
{
    return m_description;
}

/*! Sets the \a description of this DeviceDescriptor. */
void DeviceDescriptor::setDescription(const QString &description)
{
    m_description = description;
}

/*! Returns the parent device id for devices created from this DeviceDescriptor. */
DeviceId DeviceDescriptor::parentDeviceId() const
{
    return m_parentDeviceId;
}

/*! Sets the parent device Id \a parentDeviceId for devices created from this DeviceDescriptor. */
void DeviceDescriptor::setParentDeviceId(const DeviceId &parentDeviceId)
{
    m_parentDeviceId = parentDeviceId;
}

/*! Returns the list of \l{Param}{Params} of this DeviceDescriptor. */
ParamList DeviceDescriptor::params() const
{
    return m_params;
}

/*! Sets the list of \l{Param}{Params} of this DeviceDescriptor with the given \a params. */
void DeviceDescriptor::setParams(const ParamList &params)
{
    m_params = params;
}

QVariant DeviceDescriptors::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void DeviceDescriptors::put(const QVariant &variant)
{
    append(variant.value<DeviceDescriptor>());
}
