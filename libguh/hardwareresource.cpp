/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "hardwareresource.h"
#include "guhsettings.h"
#include "hardwaremanager.h"
#include "loggingcategories.h"

HardwareResource::HardwareResource(const Type &hardwareReourceType, const QString &name, QObject *parent) :
    QObject(parent),
    m_hardwareReourceType(hardwareReourceType),
    m_name(name)
{

}

QString HardwareResource::name() const
{
    return m_name;
}

bool HardwareResource::available() const
{
    return m_available;
}

bool HardwareResource::enabled() const
{
    return m_enabled;
}

HardwareResource::Type HardwareResource::hardwareReourceType() const
{
    return m_hardwareReourceType;
}

void HardwareResource::setEnabled(const bool &enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(m_enabled);
    }
}

void HardwareResource::setAvailable(const bool &available)
{
    if (m_available != available) {
        m_available = available;
        emit availableChanged(m_available);
    }
}
