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

#ifndef VENDOR_H
#define VENDOR_H

#include "libnymea.h"
#include "typeutils.h"

#include <QString>
#include <QList>

class LIBNYMEA_EXPORT Vendor
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id WRITE setId)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)

public:
    Vendor(const VendorId &id, const QString &name = QString());

    VendorId id() const;
    void setId(const VendorId &id);

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    bool operator==(const Vendor &other) const;

private:
    VendorId m_id;
    QString m_name;
    QString m_displayName;
};

class LIBNYMEA_EXPORT Vendors: public QList<Vendor>
{
public:
    Vendors();
    Vendors(const QList<Vendor> &other);

    Vendor findById(const VendorId &vendorId) const;
};

#endif // VENDOR_H
