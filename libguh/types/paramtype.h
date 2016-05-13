/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef PARAMTYPE_H
#define PARAMTYPE_H

#include <QVariant>
#include <QDebug>

#include "libguh.h"
#include "typeutils.h"

class LIBGUH_EXPORT ParamType
{
public:    
    ParamType(const QString &name, const QVariant::Type type, const QVariant &defaultValue = QVariant());

    QString name() const;
    void setName(const QString &name);

    int index() const;
    void setIndex(const int &index);

    QVariant::Type type() const;
    void setType(QVariant::Type type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

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

private:
    QString m_name;
    int m_index;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    Types::InputType m_inputType;
    Types::Unit m_unit;
    QVariantList m_allowedValues;
    bool m_readOnly;
};

QDebug operator<<(QDebug dbg, const ParamType &paramType);
QDebug operator<<(QDebug dbg, const QList<ParamType> &paramTypes);

#endif // PARAMTYPE_H
