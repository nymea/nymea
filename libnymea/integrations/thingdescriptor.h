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

#ifndef THINGDESCRIPTION_H
#define THINGDESCRIPTION_H

#include "libnymea.h"
#include "types/param.h"
#include "typeutils.h"

#include <QVariantMap>

class LIBNYMEA_EXPORT ThingDescriptor
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid thingClassId READ thingClassId)
    Q_PROPERTY(QUuid thingId READ thingId USER true)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(ParamList params READ params) // added in 0.27

public:
    ThingDescriptor();
    ThingDescriptor(const ThingClassId &thingClassId, const QString &title = QString(), const QString &description = QString(), const ThingId &parentId = ThingId());
    ThingDescriptor(const ThingDescriptorId &id,
                    const ThingClassId &thingClassId,
                    const QString &title = QString(),
                    const QString &description = QString(),
                    const ThingId &parentId = ThingId());

    bool isValid() const;

    ThingDescriptorId id() const;
    ThingClassId thingClassId() const;

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    QString title() const;
    void setTitle(const QString &title);

    QString description() const;
    void setDescription(const QString &description);

    ThingId parentId() const;
    void setParentId(const ThingId &parentId);

    ParamList params() const;
    void setParams(const ParamList &params);

private:
    ThingDescriptorId m_id;
    ThingClassId m_thingClassId;
    ThingId m_thingId;
    QString m_title;
    QString m_description;
    ThingId m_parentId;
    ParamList m_params;
};

class ThingDescriptors : public QList<ThingDescriptor>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ThingDescriptors() {}
    inline ThingDescriptors(std::initializer_list<ThingDescriptor> args)
        : QList(args)
    {}
    ThingDescriptors(const QList<ThingDescriptor> &other)
        : QList<ThingDescriptor>(other)
    {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

Q_DECLARE_METATYPE(ThingDescriptor)
Q_DECLARE_METATYPE(ThingDescriptors)

#endif // THINGDESCRIPTION_H
