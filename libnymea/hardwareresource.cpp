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
    \class HardwareResource
    \brief The base class for hardware resources.

    \inmodule libguh

    \sa HardwareResource
*/

/*! \fn HardwareResource::~HardwareResource();
    The virtual destructor of the HardwareResource.
*/

/*! \fn bool HardwareResource::available() const;
    Returns true if the hardware resource is available.

    \sa availableChanged()
*/

/*! \fn bool HardwareResource::enabled() const;
    Returns true if the hardware resource is enabled.

    \sa enabledChanged()
*/


/*! \fn bool HardwareResource::setEnabled(bool enabled);
    Sets the hardware resource to \a enabled.

    \sa enabledChanged()
*/

// Signals
/*! \fn bool HardwareResource::enabledChanged(bool enabled);
    This signal will be emitted if the hardware resource was \a enabled or disabled.
*/

/*! \fn bool HardwareResource::availableChanged(bool available);
    This signal will be emitted if the hardware resource \a available changed.
*/

#include "hardwareresource.h"
#include "hardwaremanager.h"
#include "loggingcategories.h"
#include "nymeadbusservice.h"

/*! Constructs a new HardwareResource with the given \a name and \a parent. */
HardwareResource::HardwareResource(const QString &name, QObject *parent) :
    QObject(parent),
    m_name(name)
{
    new NymeaDBusService("/io/guh/nymead/HardwareManager/" + name, this);
}

/*! Returns the name of this resource. */
QString HardwareResource::name() const
{
    return m_name;
}
