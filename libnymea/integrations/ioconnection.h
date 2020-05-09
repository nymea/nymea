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
    Q_PROPERTY(QUuid inputStateTypeId READ inputStateTypeId)
    Q_PROPERTY(QUuid outputThingId READ outputThingId)
    Q_PROPERTY(QUuid outputStateTypeId READ outputStateTypeId)
    Q_PROPERTY(bool inverted READ inverted)

public:
    IOConnection();
    IOConnection(const IOConnectionId &id, const ThingId &inputThingId, const StateTypeId &inputStateTypeId, const ThingId &outputThingId, const StateTypeId &outputStateTypeId, bool inverted = false);

    IOConnectionId id() const;

    ThingId inputThingId() const;
    StateTypeId inputStateTypeId() const;

    ThingId outputThingId() const;
    StateTypeId outputStateTypeId() const;

    bool inverted() const;

private:
    IOConnectionId m_id;
    ThingId m_inputThingId;
    StateTypeId m_inputStateTypeId;
    ThingId m_outputThingId;
    StateTypeId m_outputStateTypeId;
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
