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
    \class DeviceDescriptor
    \brief Holds the description of a \l{Device}.

    \ingroup devices
    \inmodule libnymea

    An DeviceDescriptor describes an \l{Device} in order to match it with a \l{nymeaserver::Rule}.

    \sa Device
*/


#include "thingdescriptor.h"


/*! Construct a DeviceDescriptor. */
ThingDescriptor::ThingDescriptor():
    m_id(ThingDescriptorId::createThingDescriptorId())
{

}

/*! Construct a DeviceDescriptor with the given \a thingClassId, \a title, \a description and \a parentDeviceId.*/
ThingDescriptor::ThingDescriptor(const ThingClassId &thingClassId, const QString &title, const QString &description, const ThingId &parentId):
    m_id(ThingDescriptorId::createThingDescriptorId()),
    m_thingClassId(thingClassId),
    m_title(title),
    m_description(description),
    m_parentId(parentId)
{

}

/*! Construct a DeviceDescriptor with the given \a id, \a thingClassId, \a title, \a description and \a parentDeviceId.*/
ThingDescriptor::ThingDescriptor(const ThingDescriptorId &id, const ThingClassId &thingClassId, const QString &title, const QString &description, const ThingId &parentId) :
    m_id(id),
    m_thingClassId(thingClassId),
    m_title(title),
    m_description(description),
    m_parentId(parentId)
{
}

/*! Returns true, if this DeviceDescriptor is valid. A DeviceDescriptor is valid, if the id and the thingClassId are set.
 *  \sa id(), ThingClassId(), */
bool ThingDescriptor::isValid() const
{
    return !m_id.isNull() && !m_thingClassId.isNull();
}

/*! Returns the id of this DeviceDescriptor. */
ThingDescriptorId ThingDescriptor::id() const
{
    return m_id;
}

/*! Returns the ThingClassId of this DeviceDescriptor. */
ThingClassId ThingDescriptor::thingClassId() const
{
    return m_thingClassId;
}

/*! Returns the \l {ThingId} of the thing matching this descriptor. */
ThingId ThingDescriptor::thingId() const
{
    return m_thingId;
}

/*! Set the \l {ThingId} of the thing matching this descriptor to \a{thingId}. */
void ThingDescriptor::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

/*! Returns the name of this DeviceDescriptor. */
QString ThingDescriptor::title() const
{
    return m_title;
}

/*! Sets the \a title of this DeviceDescriptor. */
void ThingDescriptor::setTitle(const QString &title)
{
    m_title = title;
}

/*! Returns the description of this DeviceDescriptor. */
QString ThingDescriptor::description() const
{
    return m_description;
}

/*! Sets the \a description of this DeviceDescriptor. */
void ThingDescriptor::setDescription(const QString &description)
{
    m_description = description;
}

/*! Returns the parent id for things created from this DeviceDescriptor. */
ThingId ThingDescriptor::parentId() const
{
    return m_parentId;
}

/*! Sets the parent id for things created from this DeviceDescriptor. */
void ThingDescriptor::setParentId(const ThingId &parentId)
{
    m_parentId = parentId;
}

/*! Returns the list of \l{Param}{Params} of this DeviceDescriptor. */
ParamList ThingDescriptor::params() const
{
    return m_params;
}

/*! Sets the list of \l{Param}{Params} of this DeviceDescriptor with the given \a params. */
void ThingDescriptor::setParams(const ParamList &params)
{
    m_params = params;
}

QVariant ThingDescriptors::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ThingDescriptors::put(const QVariant &variant)
{
    append(variant.value<ThingDescriptor>());
}
