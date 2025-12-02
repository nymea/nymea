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

#ifndef SERVICEDATA_H
#define SERVICEDATA_H

#include "typeutils.h"

#include <QDateTime>
#include <QHash>
#include <QVariant>

class ServiceData
{
public:
    ServiceData(const ThingId &thingId, const QDateTime &timestamp = QDateTime::currentDateTime());

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    QHash<QString, QVariant> data() const;
    void insert(const QString &key, const QVariant &data);
    void insert(const QHash<QString, QVariant> data);

private:
    ThingId m_thingId;
    QDateTime m_timestamp;
    QHash<QString, QVariant> m_data;
};

#endif // SERVICEDATA_H
