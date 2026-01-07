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
    Q_PROPERTY(QMetaType::Type type READ type WRITE setType)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue)
    Q_PROPERTY(Types::Unit unit READ unit WRITE setUnit USER true)
    Q_PROPERTY(Types::IOType ioType READ ioType WRITE setIOType USER true)
    Q_PROPERTY(QVariant minValue READ minValue WRITE setMinValue USER true)
    Q_PROPERTY(QVariant maxValue READ maxValue WRITE setMaxValue USER true)
    Q_PROPERTY(double stepSize READ stepSize WRITE setStepSize USER true)
    Q_PROPERTY(QVariantList possibleValues READ possibleValues WRITE setPossibleValues USER true)
    Q_PROPERTY(QStringList possibleValuesDisplayNames READ possibleValuesDisplayNames WRITE setPossibleValuesDisplayNames USER true)

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

    QMetaType::Type type() const;
    void setType(const QMetaType::Type &type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

    double stepSize() const;
    void setStepSize(double stepSize);

    QVariantList possibleValues() const;
    void setPossibleValues(const QVariantList &possibleValues);

    QStringList possibleValuesDisplayNames() const;
    void setPossibleValuesDisplayNames(const QStringList &possibleValuesDisplayNames);

    Types::Unit unit() const;
    void setUnit(const Types::Unit &unit);

    Types::IOType ioType() const;
    void setIOType(Types::IOType ioType);

    bool writable() const;
    void setWritable(bool writable);

    bool cached() const;
    void setCached(bool cached);

    bool suggestLogging() const;
    void setSuggestLogging(bool suggestLogging);

    Types::StateValueFilter filter() const;
    void setFilter(Types::StateValueFilter filter);

    bool isValid() const;

private:
    StateTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index = 0;
    QMetaType::Type m_type = QMetaType::UnknownType;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    double m_stepSize = 0;
    QVariantList m_possibleValues;
    QStringList m_possibleValuesDisplayNames;
    Types::Unit m_unit = Types::UnitNone;
    Types::IOType m_ioType = Types::IOTypeNone;
    bool m_writable = false;
    bool m_cached = true;
    bool m_logged = false;
    Types::StateValueFilter m_filter = Types::StateValueFilterNone;
};
Q_DECLARE_METATYPE(StateType)

class StateTypes : public QList<StateType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    StateTypes() = default;
    StateTypes(const QList<StateType> &other);
    bool contains(const StateTypeId &stateTypeId);
    bool contains(const QString &name);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    StateType findByName(const QString &name);
    StateType findById(const StateTypeId &id);
    StateType &operator[](const QString &name);
};
Q_DECLARE_METATYPE(StateTypes)

#endif // STATETYPE_H
