/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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

/*! Constructs an ParamDescriptor describing an \l{Param} with the given \a paramTypeId and \a value.
 *  The ValueOperator is by default ValueOperatorEquals. */
ParamDescriptor::ParamDescriptor(const ParamTypeId &paramTypeId, const QVariant &value):
    Param(paramTypeId, value),
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

