/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "paramtype.h"

ParamType::ParamType(const QString &name, const QVariant::Type type, const QVariant &defaultValue):
    m_name(name),
    m_type(type),
    m_defaultValue(defaultValue)
{
}

QString ParamType::name() const
{
    return m_name;
}

void ParamType::setName(const QString &name)
{
    m_name = name;
}

QVariant::Type ParamType::type() const
{
    return m_type;
}

void ParamType::setType(QVariant::Type type)
{
    m_type = type;
}

QVariant ParamType::defaultValue() const
{
    return m_defaultValue;
}

void ParamType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}

QVariant ParamType::minValue() const
{
    return m_minValue;
}

void ParamType::setMinValue(const QVariant &minValue)
{
    m_minValue = minValue;
}

QVariant ParamType::maxValue() const
{
    return m_maxValue;
}

void ParamType::setMaxValue(const QVariant &maxValue)
{
    m_maxValue = maxValue;
}

QPair<QVariant, QVariant> ParamType::limits() const
{
    return qMakePair<QVariant, QVariant>(m_minValue, m_maxValue);
}

void ParamType::setLimits(const QVariant &min, const QVariant &max)
{
    m_minValue = min;
    m_maxValue = max;
}

QList<QVariant> ParamType::allowedValues() const
{
    return m_allowedValues;
}

void ParamType::setAllowedValues(const QList<QVariant> allowedValues)
{
    m_allowedValues = allowedValues;
}

QDebug operator<<(QDebug dbg, const ParamType &paramType)
{
    dbg.nospace() << "ParamType(Name: " << paramType.name()
                  << ", Type:" << QVariant::typeToName(paramType.type())
                  << ", Default:" << paramType.defaultValue()
                  << ", Min:" << paramType.minValue()
                  << ", Max:" << paramType.maxValue()
                  << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QList<ParamType> &paramTypes)
{
    dbg.nospace() << "ParamTypeList (count:" << paramTypes.count() << ")" << endl;
    for (int i = 0; i < paramTypes.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << paramTypes.at(i) << endl;
    }

    return dbg.space();
}
