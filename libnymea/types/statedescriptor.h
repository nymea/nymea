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
    Q_GADGET
    Q_PROPERTY(QUuid stateTypeId READ stateTypeId WRITE setStateTypeId USER true)
    Q_PROPERTY(QUuid deviceId READ deviceId WRITE setDeviceId USER true)
    Q_PROPERTY(QString interface READ interface WRITE setInterface USER true)
    Q_PROPERTY(QString interfaceState READ interfaceState WRITE setInterfaceState USER true)
    Q_PROPERTY(QVariant value READ stateValue WRITE setStateValue)
    Q_PROPERTY(Types::ValueOperator operator READ operatorType WRITE setOperatorType)
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
    void setStateTypeId(const StateTypeId &stateTypeId);

    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    QString interface() const;
    void setInterface(const QString &interface);

    QString interfaceState() const;
    void setInterfaceState(const QString &interfaceState);

    QVariant stateValue() const;
    void setStateValue(const QVariant &value);

    Types::ValueOperator operatorType() const;
    void setOperatorType(Types::ValueOperator opertatorType);

    Q_INVOKABLE bool isValid() const;

    bool operator ==(const StateDescriptor &other) const;

    bool operator ==(const State &state) const;
    bool operator !=(const State &state) const;

private:
    StateTypeId m_stateTypeId;
    DeviceId m_deviceId;
    QString m_interface;
    QString m_interfaceState;
    QVariant m_stateValue;
    Types::ValueOperator m_operatorType = Types::ValueOperatorEquals;
};
Q_DECLARE_METATYPE(StateDescriptor)

QDebug operator<<(QDebug dbg, const StateDescriptor &stateDescriptor);

#endif // STATEDESCRIPTOR_H
