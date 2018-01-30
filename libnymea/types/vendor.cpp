/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

/*!
    \class Vendor
    \brief Holds information about a the vendor of a \l{Device}.

    \ingroup guh-types
    \inmodule libnymea

    \sa DevicePlugin
*/

#include "vendor.h"

/*! Constructs an Vendor with the given \a id and the given \a name. */
Vendor::Vendor(const VendorId &id, const QString &name):
    m_id(id),
    m_name(name)
{
}

/*! Returns the id of this Vendor. */
VendorId Vendor::id() const
{
    return m_id;
}
/*! Set the id of this Vendor with the given \a id. */
void Vendor::setId(const VendorId &id)
{
    m_id = id;
}

/*! Returns the name of this Vendor. */
QString Vendor::name() const
{
    return m_name;
}

/*! Set the name of this Vendor with the given \a name. */
void Vendor::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the name of this Vendor, to be shown to the user, translated. */
QString Vendor::displayName() const
{
    return m_displayName;
}

/*! Sets the name of this Vendor, to be shown to the user, translated. */
void Vendor::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}
