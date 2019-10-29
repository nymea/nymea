/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

#ifndef PARAMDESCRIPTOR_H
#define PARAMDESCRIPTOR_H

#include "libnymea.h"

#include "param.h"
#include "typeutils.h"

class LIBNYMEA_EXPORT ParamDescriptor : public Param
{
    Q_GADGET
    Q_PROPERTY(QString paramName READ paramName WRITE setParamName USER true)
    Q_PROPERTY(Types::ValueOperator operator READ operatorType WRITE setOperatorType)
public:
    ParamDescriptor() = default;
    ParamDescriptor(const ParamTypeId &paramTypeId, const QVariant &value = QVariant());
    ParamDescriptor(const QString &paramName, const QVariant &value = QVariant());

    QString paramName() const;
    void setParamName(const QString &paramName);

    Types::ValueOperator operatorType() const;
    void setOperatorType(Types::ValueOperator operatorType);

private:
    QString m_paramName;
    Types::ValueOperator m_operatorType;
};
Q_DECLARE_METATYPE(ParamDescriptor)

class LIBNYMEA_EXPORT ParamDescriptors: public QList<ParamDescriptor>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ParamDescriptors();
    ParamDescriptors(const QList<ParamDescriptor> &other);
    Q_INVOKABLE QVariant get(int index) const;
};
Q_DECLARE_METATYPE(ParamDescriptors)

QDebug operator<<(QDebug dbg, const ParamDescriptor &paramDescriptor);

#endif // PARAMDESCRIPTOR_H
