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

#ifndef ACTIONTYPE_H
#define ACTIONTYPE_H

#include "libnymea.h"
#include "typeutils.h"
#include "paramtype.h"

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

    static QStringList typeProperties();
    static QStringList mandatoryTypeProperties();

private:
    ActionTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index;
    ParamTypes m_paramTypes;
};
Q_DECLARE_METATYPE(ActionType)

QDebug operator<<(QDebug dbg, const ActionType &actionType);

class ActionTypes: public QList<ActionType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ActionTypes() = default;
    ActionTypes(const QList<ActionType> &other);
    Q_INVOKABLE QVariant get(int index);
    ActionType findByName(const QString &name);
    ActionType findById(const ActionTypeId &id);
};
Q_DECLARE_METATYPE(ActionTypes)

#endif // ACTIONTYPE_H
