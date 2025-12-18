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

#ifndef ACTIONTYPE_H
#define ACTIONTYPE_H

#include "libnymea.h"
#include "paramtype.h"
#include "typeutils.h"

#include <QVariantList>

class LIBNYMEA_EXPORT ActionType
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(ParamTypes paramTypes READ paramTypes WRITE setParamTypes)

public:
    ActionType(const ActionTypeId &id = ActionTypeId());

    ActionTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int index() const;
    void setIndex(const int &index);

    ParamTypes paramTypes() const;
    void setParamTypes(const ParamTypes &paramTypes);

private:
    ActionTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index;
    ParamTypes m_paramTypes;
};
Q_DECLARE_METATYPE(ActionType)

QDebug operator<<(QDebug dbg, const ActionType &actionType);

class ActionTypes : public QList<ActionType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ActionTypes() = default;
    ActionTypes(const QList<ActionType> &other);
    bool contains(const ActionTypeId &id) const;
    bool contains(const QString &name) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    ActionType findByName(const QString &name);
    ActionType findById(const ActionTypeId &id);
    ActionType &operator[](const QString &name);
};
Q_DECLARE_METATYPE(ActionTypes)

#endif // ACTIONTYPE_H
