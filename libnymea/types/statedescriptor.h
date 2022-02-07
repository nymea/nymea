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
    Q_PROPERTY(QUuid thingId READ thingId WRITE setThingId USER true)
    Q_PROPERTY(QString interface READ interface WRITE setInterface USER true)
    Q_PROPERTY(QString interfaceState READ interfaceState WRITE setInterfaceState USER true)
    Q_PROPERTY(QVariant value READ stateValue WRITE setStateValue USER true)
    Q_PROPERTY(QUuid valueThingId READ valueThingId WRITE setValueThingId USER true)
    Q_PROPERTY(QUuid valueStateTypeId READ valueStateTypeId WRITE setValueStateTypeId USER true)
    Q_PROPERTY(Types::ValueOperator operator READ operatorType WRITE setOperatorType)
public:
    enum Type {
        TypeThing,
        TypeInterface
    };

    StateDescriptor();
    StateDescriptor(const StateTypeId &stateTypeId, const ThingId &thingId, const QVariant &stateValue, Types::ValueOperator operatorType = Types::ValueOperatorEquals);
    StateDescriptor(const QString &interface, const QString &interfaceState, const QVariant &stateValue, Types::ValueOperator operatorType = Types::ValueOperatorEquals);

    Type type() const;

    StateTypeId stateTypeId() const;
    void setStateTypeId(const StateTypeId &stateTypeId);

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    QString interface() const;
    void setInterface(const QString &interface);

    QString interfaceState() const;
    void setInterfaceState(const QString &interfaceState);

    QVariant stateValue() const;
    void setStateValue(const QVariant &value);

    ThingId valueThingId() const;
    void setValueThingId(const ThingId &valueThingId);

    StateTypeId valueStateTypeId() const;
    void setValueStateTypeId(const StateTypeId &valueStateTypeId);

    Types::ValueOperator operatorType() const;
    void setOperatorType(Types::ValueOperator opertatorType);

    Q_INVOKABLE bool isValid() const;

    bool operator ==(const StateDescriptor &other) const;

private:
    StateTypeId m_stateTypeId;
    ThingId m_thingId;
    QString m_interface;
    QString m_interfaceState;
    QVariant m_stateValue;
    ThingId m_valueThingId;
    StateTypeId m_valueStateTypeId;
    Types::ValueOperator m_operatorType = Types::ValueOperatorEquals;
};
Q_DECLARE_METATYPE(StateDescriptor)

QDebug operator<<(QDebug dbg, const StateDescriptor &stateDescriptor);

#endif // STATEDESCRIPTOR_H
