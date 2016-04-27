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

/*!
    \class Param
    \brief Holds the parameters of a Param.

    \ingroup guh-types
    \inmodule libguh

    \sa Device, ParamType, ParamDescriptor
*/

#include "param.h"

#include <QDebug>

/*! Constructs a Param with the given \a name and \a value of the paramter. */
Param::Param(const QString &name, const QVariant &value):
    m_name (name),
    m_value(value)
{
}

/*! Returns the name of this Param. */
QString Param::name() const
{
    return m_name;
}

/*! Sets the \a name of this Param. */
void Param::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the value of this Param. */
QVariant Param::value() const
{
    return m_value;
}

/*! Sets the \a value of this Param. */
void Param::setValue(const QVariant &value)
{
    m_value = value;
}

/*! A Param is valid if name and and value are set. Returns true if valid, false if not. */
bool Param::isValid() const
{
    return !m_name.isEmpty() && m_value.isValid();
}

/*! Writes the name and value of the given \a param to \a dbg. */
QDebug operator<<(QDebug dbg, const Param &param)
{
    dbg.nospace() << "Param(Name: " << param.name() << ", Value:" << param.value() << ")";

    return dbg.space();
}

/*! Writes the param of the given \a params to \a dbg. */
QDebug operator<<(QDebug dbg, const ParamList &params)
{
    dbg.nospace() << "ParamList (count:" << params.count() << ")" << endl;
    for (int i = 0; i < params.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << params.at(i) << endl;
    }

    return dbg.space();
}

/*!
    \class ParamList
    \brief Holds a list of \l{Param}{Params}

    \ingroup types
    \inmodule libguh

    \sa Param,
*/

/*! Returns true if this Param contains a Param with the given \a paramName. */
bool ParamList::hasParam(const QString &paramName) const
{
    foreach (const Param &param, *this) {
        if (param.name() == paramName) {
            return true;
        }
    }
    return false;
}

/*! Returns the value of the Param with the given \a paramName. */
QVariant ParamList::paramValue(const QString &paramName) const
{
    foreach (const Param &param, *this) {
        if (param.name() == paramName) {
            return param.value();
        }
    }
    return QVariant();
}

/*! Sets the value of a Param with the given \a paramName to the given \a value. */
void ParamList::setParamValue(const QString &paramName, const QVariant &value)
{
    for (int i = 0; i < count(); i++) {
        if (this->operator [](i).name()  == paramName) {
            this->operator [](i).setValue(value);
            return;
        }
    }
}

/*! Appends the given \a param to a ParamList. */
ParamList ParamList::operator<<(const Param &param)
{
    this->append(param);
    return *this;
}
