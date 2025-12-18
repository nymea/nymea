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

/*!
    \class Param
    \brief Holds the parameters of a Param.

    \ingroup nymea-types
    \inmodule libnymea

    \sa Device, ParamType, ParamDescriptor
*/

#include "param.h"

#include <QDebug>

/*! Constructs a \l Param with the given \a paramTypeId and \a value of the parameter. */
Param::Param(const ParamTypeId &paramTypeId, const QVariant &value)
    : m_paramTypeId(paramTypeId)
    , m_value(value)
{}

/*! Returns the paramTypeId of this \l Param. */
ParamTypeId Param::paramTypeId() const
{
    return m_paramTypeId;
}

void Param::setParamTypeId(const ParamTypeId &paramTypeId)
{
    m_paramTypeId = paramTypeId;
}

/*! Returns the value of this \l Param. */
QVariant Param::value() const
{
    return m_value;
}

/*! Sets the \a value of this \l Param. */
void Param::setValue(const QVariant &value)
{
    m_value = value;
}

/*! Returns true if paramTypeId() and and value() of this \l Param are set. */
bool Param::isValid() const
{
    return !m_paramTypeId.isNull() && m_value.isValid();
}

/*! Writes the paramTypeId and value of the given \a param to \a dbg. */
QDebug operator<<(QDebug dbg, const Param &param)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Param(Id: " << param.paramTypeId().toString() << ", Value: " << param.value() << ")";
    return dbg;
}

/*! Writes the param of the given \a params to \a dbg. */
QDebug operator<<(QDebug dbg, const ParamList &params)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "ParamList (count:" << params.count() << ")";
    if (params.count() == 0) {
        dbg.nospace() << '\n';
    }
    for (int i = 0; i < params.count(); i++) {
        dbg.nospace() << '\n' << "     " << i << ": " << params.at(i);
    }

    return dbg;
}

/*!
    \class ParamList
    \brief Holds a list of \l{Param}{Params}

    \ingroup types
    \inmodule libnymea

    \sa Param,
*/

/*! Constructs an empty ParamList. */
ParamList::ParamList() {}

/*! Constructs a ParamList from a QList<Param>. */
ParamList::ParamList(const QList<Param> &other)
    : QList<Param>(other)
{}

QVariant ParamList::get(int index)
{
    return QVariant::fromValue(at(index));
}

void ParamList::put(const QVariant &variant)
{
    append(variant.value<Param>());
}

/*! Returns true if this ParamList contains a Param with the given \a paramTypeId. */
bool ParamList::hasParam(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, *this) {
        if (param.paramTypeId() == paramTypeId)
            return true;
    }

    return false;
}

/*! Returns the value of the Param with the given \a paramTypeId. */
QVariant ParamList::paramValue(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, *this) {
        if (param.paramTypeId() == paramTypeId)
            return param.value();
    }

    return QVariant();
}

/*! Returns true if the value of a Param with the given \a paramTypeId could be set to the given \a value. */
bool ParamList::setParamValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    for (int i = 0; i < count(); i++) {
        if (this->operator[](i).paramTypeId() == paramTypeId) {
            this->operator[](i).setValue(value);
            return true;
        }
    }

    return false;
}

/*! Appends the given \a param to a ParamList. */
ParamList ParamList::operator<<(const Param &param)
{
    this->append(param);
    m_ids.append(param.paramTypeId());
    return *this;
}
