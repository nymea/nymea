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
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(ParamDescriptors)

QDebug operator<<(QDebug dbg, const ParamDescriptor &paramDescriptor);

#endif // PARAMDESCRIPTOR_H
