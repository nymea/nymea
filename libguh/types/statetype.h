/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef STATETYPE_H
#define STATETYPE_H

#include "libguh.h"
#include "typeutils.h"

#include <QVariant>

class LIBGUH_EXPORT StateType
{
public:
    StateType(const StateTypeId &id);

    StateTypeId id() const;

    QString name() const;
    void setName(const QString &name);

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

private:
    StateTypeId m_id;
    QString m_name;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    QVariantList m_possibleValues;
    Types::Unit m_unit;
};

#endif // STATETYPE_H
