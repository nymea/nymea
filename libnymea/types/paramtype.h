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

#ifndef PARAMTYPE_H
#define PARAMTYPE_H

#include <QDebug>
#include <QVariant>

#include "libnymea.h"
#include "typeutils.h"

class LIBNYMEA_EXPORT ParamType
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)
    Q_PROPERTY(QMetaType::Type type READ type WRITE setType)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue USER true)
    Q_PROPERTY(QVariant minValue READ minValue WRITE setMinValue USER true)
    Q_PROPERTY(QVariant maxValue READ maxValue WRITE setMaxValue USER true)
    Q_PROPERTY(double stepSize READ stepSize WRITE setStepSize USER true)
    Q_PROPERTY(QVariantList allowedValues READ allowedValues WRITE setAllowedValues USER true)
    Q_PROPERTY(Types::InputType inputType READ inputType WRITE setInputType USER true)
    Q_PROPERTY(Types::Unit unit READ unit WRITE setUnit USER true)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly USER true)

public:
    ParamType() = default;
    ParamType(const ParamTypeId &id, const QString &name, const QMetaType::Type type, const QVariant &defaultValue = QVariant());

    ParamTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int index() const;
    void setIndex(const int &index);

    QMetaType::Type type() const;
    void setType(QMetaType::Type type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

    double stepSize() const;
    void setStepSize(double stepSize);

    Types::InputType inputType() const;
    void setInputType(const Types::InputType &inputType);

    Types::Unit unit() const;
    void setUnit(const Types::Unit &unit);

    QPair<QVariant, QVariant> limits() const;
    void setLimits(const QVariant &min, const QVariant &max);

    QList<QVariant> allowedValues() const;
    void setAllowedValues(const QList<QVariant> &allowedValues);

    bool readOnly() const;
    void setReadOnly(const bool &readOnly);

    bool isValid() const;

    static QStringList typeProperties();
    static QStringList mandatoryTypeProperties();

private:
    ParamTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index = 0;
    QMetaType::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    double m_stepSize = 0;
    Types::InputType m_inputType = Types::InputTypeNone;
    Types::Unit m_unit = Types::UnitNone;
    QVariantList m_allowedValues;
    bool m_readOnly = false;
};

class ParamTypes : public QList<ParamType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ParamTypes() = default;
    ParamTypes(const QList<ParamType> &other);
    bool contains(const ParamTypeId &paramTypeId);
    bool contains(const QString &name);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    ParamType findByName(const QString &name) const;
    ParamType findById(const ParamTypeId &id) const;
    ParamType &operator[](const QString &name);
};
Q_DECLARE_METATYPE(QList<ParamType>)
Q_DECLARE_METATYPE(ParamTypes)

QDebug operator<<(QDebug dbg, const ParamType &paramType);
QDebug operator<<(QDebug dbg, const QList<ParamType> &paramTypes);

#endif // PARAMTYPE_H
