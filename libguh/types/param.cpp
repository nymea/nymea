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

#include "param.h"

#include <QDebug>

Param::Param(const QString &name, const QVariant &value):
    m_name (name),
    m_value(value)
{
}

QString Param::name() const
{
    return m_name;
}

void Param::setName(const QString &name)
{
    m_name = name;
}

QVariant Param::value() const
{
    return m_value;
}

void Param::setValue(const QVariant &value)
{
    m_value = value;
}

bool Param::isValid() const
{
    return !m_name.isEmpty() && m_value.isValid();
}

QDebug operator<<(QDebug dbg, const Param &param)
{
    dbg.nospace() << "Param(Name: " << param.name() << ", Value:" << param.value() << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QList<Param> &params)
{
    dbg.nospace() << "ParamList (count:" << params.count() << ")" << endl;
    for (int i = 0; i < params.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << params.at(i) << endl;
    }

    return dbg.space();
}
