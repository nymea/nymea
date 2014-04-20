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

#include "devicedescriptor.h"

DeviceDescriptor::DeviceDescriptor():
    m_id(DeviceDescriptorId::createDeviceDescriptorId())
{

}

DeviceDescriptor::DeviceDescriptor(const DeviceClassId &deviceClassId, const QString &title, const QString &description):
    m_id(DeviceDescriptorId::createDeviceDescriptorId()),
    m_deviceClassId(deviceClassId),
    m_title(title),
    m_description(description)
{

}

DeviceDescriptor::DeviceDescriptor(const DeviceDescriptorId &id, const DeviceClassId &deviceClassId, const QString &title, const QString &description) :
    m_id(id),
    m_deviceClassId(deviceClassId),
    m_title(title),
    m_description(description)
{
}

bool DeviceDescriptor::isValid() const
{
    return !m_id.isNull() && !m_deviceClassId.isNull();
}

DeviceDescriptorId DeviceDescriptor::id() const
{
    return m_id;
}

DeviceClassId DeviceDescriptor::deviceClassId() const
{
    return m_deviceClassId;
}

QString DeviceDescriptor::title() const
{
    return m_title;
}

void DeviceDescriptor::setTitle(const QString &title)
{
    m_title = title;
}

QString DeviceDescriptor::description() const
{
    return m_description;
}

void DeviceDescriptor::setDescription(const QString &description)
{
    m_description = description;
}

QList<Param> DeviceDescriptor::params() const
{
    return m_params;
}

void DeviceDescriptor::setParams(const QList<Param> &params)
{
    m_params = params;
}
