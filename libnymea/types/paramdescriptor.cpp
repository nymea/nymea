/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class ParamDescriptor
    \brief Describes a certain \l{Param}.

    \ingroup nymea-types
    \ingroup rules
    \inmodule libnymea

    An ParamDescriptor describes a \l{Param} in order to match it with a \l{nymeaserver::Rule}.

    \sa Param, ParamType
*/

#include "paramdescriptor.h"

#include <QDebug>

/*! Constructs an ParamDescriptor describing an \l{Param} with the given \a paramTypeId and \a value.
 *  The ValueOperator is by default ValueOperatorEquals. */
ParamDescriptor::ParamDescriptor(const ParamTypeId &paramTypeId, const QVariant &value):
    Param(paramTypeId, value),
    m_operatorType(Types::ValueOperatorEquals)
{
}

/*! Constructs an ParamDescriptor describing an \l{Param} with the given \a paramName and \a value.
 *  The ValueOperator is by default ValueOperatorEquals. */
ParamDescriptor::ParamDescriptor(const QString &paramName, const QVariant &value):
    Param(ParamTypeId(), value),
    m_paramName(paramName),
    m_operatorType(Types::ValueOperatorEquals)
{

}

/*! Returns the paramName of this ParamDescriptor. */
QString ParamDescriptor::paramName() const
{
    return m_paramName;
}

/*! Sets the param name of this ParamDescriptor. */
void ParamDescriptor::setParamName(const QString &paramName)
{
    m_paramName = paramName;
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

/*! Print a ParamDescriptor to QDebug. */
QDebug operator<<(QDebug dbg, const ParamDescriptor &paramDescriptor)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "ParamDescriptor(ParamTypeId: " << paramDescriptor.paramTypeId().toString() << ", Name:" << paramDescriptor.paramName() << ", Value:" << paramDescriptor.value() << ")" << endl;
    return dbg;
}

ParamDescriptors::ParamDescriptors()
{

}

ParamDescriptors::ParamDescriptors(const QList<ParamDescriptor> &other): QList<ParamDescriptor>(other)
{

}

QVariant ParamDescriptors::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ParamDescriptors::put(const QVariant &variant)
{
    append(variant.value<ParamDescriptor>());
}
