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

#ifndef STATEDESCRIPTOR_H
#define STATEDESCRIPTOR_H

#include "libguh.h"
#include "typeutils.h"
#include "paramdescriptor.h"
#include "state.h"
#include "event.h"

#include <QString>
#include <QVariantList>
#include <QDebug>

class LIBGUH_EXPORT StateDescriptor
{
public:
    StateDescriptor();
    StateDescriptor(const StateTypeId &stateTypeId, const DeviceId &deviceId, const QVariant &stateValue, Types::ValueOperator operatorType = Types::ValueOperatorEquals);

    StateTypeId stateTypeId() const;
    DeviceId deviceId() const;
    QVariant stateValue() const;
    Types::ValueOperator operatorType() const;

    bool isValid() const;

    bool operator ==(const StateDescriptor &other) const;

    bool operator ==(const State &state) const;
    bool operator !=(const State &state) const;

private:
    StateTypeId m_stateTypeId;
    DeviceId m_deviceId;
    QVariant m_stateValue;
    Types::ValueOperator m_operatorType;
};

QDebug operator<<(QDebug dbg, const StateDescriptor &eventDescriptor);
QDebug operator<<(QDebug dbg, const QList<StateDescriptor> &eventDescriptors);

#endif // STATEDESCRIPTOR_H
