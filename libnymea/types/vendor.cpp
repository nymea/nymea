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
    \class Vendor
    \brief Holds information about a the vendor of a \l{Device}.

    \ingroup nymea-types
    \inmodule libnymea

    \sa DevicePlugin
*/

#include "vendor.h"

Vendor::Vendor()
{

}

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

/*! Sets the \a displayName of this Vendor, to be shown to the user, translated. */
void Vendor::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

bool Vendor::operator==(const Vendor &other) const
{
    return m_id == other.id();
}

Vendors::Vendors()
{

}

Vendors::Vendors(const QList<Vendor> &other): QList<Vendor>(other)
{

}

QVariant Vendors::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Vendors::put(const QVariant &variant)
{
    append(variant.value<Vendor>());
}

Vendor Vendors::findById(const VendorId &vendorId) const
{
    foreach (const Vendor &vendor, *this) {
        if (vendor.id() == vendorId) {
            return vendor;
        }
    }
    return Vendor(VendorId());
}
