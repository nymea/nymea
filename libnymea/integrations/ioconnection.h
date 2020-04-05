#ifndef IOCONNECTION_H
#define IOCONNECTION_H

#include <QObject>
#include <QList>
#include <QVariant>

#include "typeutils.h"

class IOConnection
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid inputThingId READ inputThingId)
    Q_PROPERTY(QUuid inputStateTypeId READ inputStateTypeId)
    Q_PROPERTY(QUuid outputThingId READ outputThingId)
    Q_PROPERTY(QUuid outputStateTypeId READ outputStateTypeId)

public:
    IOConnection();
    IOConnection(const IOConnectionId &id, const ThingId &inputThingId, const StateTypeId &inputStateTypeId, const ThingId &outputThingId, const StateTypeId &outputStateTypeId);

    IOConnectionId id() const;

    ThingId inputThingId() const;
    StateTypeId inputStateTypeId() const;

    ThingId outputThingId() const;
    StateTypeId outputStateTypeId() const;

private:
    IOConnectionId m_id;
    ThingId m_inputThingId;
    StateTypeId m_inputStateTypeId;
    ThingId m_outputThingId;
    StateTypeId m_outputStateTypeId;
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
