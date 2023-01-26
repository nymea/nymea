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

#ifndef IOCONNECTION_H
#define IOCONNECTION_H

#include <QObject>
#include <QList>
#include <QVariant>

#include "typeutils.h"
#include "thing.h"

struct IOConnectionResult {
    Thing::ThingError error = Thing::ThingErrorNoError;
    IOConnectionId ioConnectionId;
};

class IOConnection
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid inputThingId READ inputThingId)
    Q_PROPERTY(QUuid inputStateTypeId READ inputStateTypeId USER true REVISION 1)
    Q_PROPERTY(QString inputState READ inputState USER true) // TODO: Make mandatory when inputStateTypeId is removed
    Q_PROPERTY(QUuid outputThingId READ outputThingId)
    Q_PROPERTY(QUuid outputStateTypeId READ outputStateTypeId USER true REVISION 1)
    Q_PROPERTY(QString outputState READ outputState USER true) // TODO: Make mandatory when outputStateTypeId is removed
    Q_PROPERTY(bool inverted READ inverted)

public:
    IOConnection();
    IOConnection(const IOConnectionId &id, const ThingId &inputThingId, const QString &inputState, const ThingId &outputThingId, const QString &outputState, bool inverted = false);

    IOConnectionId id() const;

    ThingId inputThingId() const;
    QString inputState() const;
    StateTypeId inputStateTypeId() const;
    void setInputStateTypeId(const StateTypeId &stateTypeId); // Deprecated

    ThingId outputThingId() const;
    StateTypeId outputStateTypeId() const;
    void setOutputStateTypeId(const StateTypeId &stateTypeId); // Deprecated
    QString outputState() const;

    bool inverted() const;

private:
    IOConnectionId m_id;
    ThingId m_inputThingId;
    StateTypeId m_inputStateTypeId; // Deprecated
    QString m_inputState;
    ThingId m_outputThingId;
    StateTypeId m_outputStateTypeId; // Deprecated
    QString m_outputState;
    bool m_inverted = false;
};

class IOConnections: public QList<IOConnection>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    IOConnections() {}
    inline IOConnections(std::initializer_list<IOConnection> args): QList(args) {}
    IOConnections(const QList<IOConnection> &other): QList<IOConnection>(other) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

Q_DECLARE_METATYPE(IOConnection)
Q_DECLARE_METATYPE(IOConnections)


#endif // IOCONNECTION_H
