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


#ifndef STATETYPE_H
#define STATETYPE_H

#include "libnymea.h"
#include "typeutils.h"

#include <QVariant>

class LIBNYMEA_EXPORT StateType
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)
    Q_PROPERTY(QVariant::Type type READ type WRITE setType)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue)
    Q_PROPERTY(Types::Unit unit READ unit WRITE setUnit USER true)
    Q_PROPERTY(QVariant minValue READ minValue WRITE setMinValue USER true)
    Q_PROPERTY(QVariant maxValue READ maxValue WRITE setMaxValue USER true)
    Q_PROPERTY(QVariantList possibleValues READ possibleValues WRITE setPossibleValues USER true)

public:
    StateType();
    StateType(const StateTypeId &id);

    StateTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int index() const;
    void setIndex(const int &index);

    QVariant::Type type() const;
    void setType(const QVariant::Type &type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

    QVariantList possibleValues() const;
    void setPossibleValues(const QVariantList &possibleValues);

    Types::Unit unit() const;
    void setUnit(const Types::Unit &unit);

    bool cached() const;
    void setCached(bool cached);

    static QStringList typeProperties();
    static QStringList mandatoryTypeProperties();

private:
    StateTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index = 0;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    QVariantList m_possibleValues;
    Types::Unit m_unit = Types::UnitNone;
    bool m_cached = true;
};
Q_DECLARE_METATYPE(StateType)

class StateTypes: public QList<StateType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    StateTypes() = default;
    StateTypes(const QList<StateType> &other);
    Q_INVOKABLE QVariant get(int index);
    StateType findByName(const QString &name);
    StateType findById(const StateTypeId &id);
};
Q_DECLARE_METATYPE(StateTypes)

#endif // STATETYPE_H
