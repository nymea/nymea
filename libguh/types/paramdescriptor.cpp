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

/*!
    \class ParamDescriptor
    \brief Describes a certain \l{Param}.

    \ingroup guh-types
    \ingroup rules
    \inmodule libguh

    An ParamDescriptor describes a \l{Param} in order to match it with a \l{guhserver::Rule}.

    \sa Param, ParamType
*/

#include "paramdescriptor.h"

/*! Constructs an ParamDescriptor describing an \l{Param} with the given \a name and \a value.
 *  The ValueOperator is by default ValueOperatorEquals. */
ParamDescriptor::ParamDescriptor(const QString &name, const QVariant &value):
    Param(name, value),
    m_operatorType(Types::ValueOperatorEquals)
{
}

/*! Returns the ValueOperator of this ParamDescriptor. */
Types::ValueOperator ParamDescriptor::operatorType() const
{
    return m_operatorType;
}

/*! Sets the ValueOperator of this ParamDescriptor to the given \a operatorType. */
void ParamDescriptor::setOperatorType(Types::ValueOperator operatorType)
{
    m_operatorType = operatorType;
}

