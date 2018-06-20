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

#ifndef STATEDESCRIPTOR_H
#define STATEDESCRIPTOR_H

#include "libnymea.h"
#include "typeutils.h"
#include "paramdescriptor.h"
#include "state.h"
#include "event.h"

#include <QString>
#include <QVariantList>
#include <QDebug>

class LIBNYMEA_EXPORT StateDescriptor
{
public:
    enum Type {
        TypeDevice,
        TypeInterface
    };

    StateDescriptor();
    StateDescriptor(const StateTypeId &stateTypeId, const DeviceId &deviceId, const QVariant &stateValue, Types::ValueOperator operatorType = Types::ValueOperatorEquals);
    StateDescriptor(const QString &interface, const QString &interfaceState, const QVariant &stateValue, Types::ValueOperator operatorType = Types::ValueOperatorEquals);

    Type type() const;

    StateTypeId stateTypeId() const;
    DeviceId deviceId() const;

    QString interface() const;
    QString interfaceState() const;

    QVariant stateValue() const;
    Types::ValueOperator operatorType() const;

    bool isValid() const;

    bool operator ==(const StateDescriptor &other) const;

    bool operator ==(const State &state) const;
    bool operator !=(const State &state) const;

private:
    StateTypeId m_stateTypeId;
    DeviceId m_deviceId;
    QString m_interface;
    QString m_interfaceState;
    QVariant m_stateValue;
    Types::ValueOperator m_operatorType;
};

QDebug operator<<(QDebug dbg, const StateDescriptor &stateDescriptor);

#endif // STATEDESCRIPTOR_H
