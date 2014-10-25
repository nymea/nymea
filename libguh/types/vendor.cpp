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

/*!
    \class Vendor
    \brief Holds information about a the vendor of a \l{Device}.

    \ingroup types
    \inmodule libguh

    \sa Device
*/

#include "vendor.h"

/*! Constructs an Vendor with the given \a id and the given \a name.
 */
Vendor::Vendor(const VendorId &id, const QString &name):
    m_id(id),
    m_name(name)
{
}

/*! Returns the id of this vendor. */
VendorId Vendor::id() const
{
    return m_id;
}
/*! Set the id of this vendor with the given \a id. */
void Vendor::setId(const VendorId &id)
{
    m_id = id;
}

/*! Returns the name of this vendor. */
QString Vendor::name() const
{
    return m_name;
}

/*! Set the name of this vendor with the given \a name. */
void Vendor::setName(const QString &name)
{
    m_name = name;
}
