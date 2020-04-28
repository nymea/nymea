#include "ioconnection.h"

IOConnection::IOConnection()
{

}

IOConnection::IOConnection(const IOConnectionId &id, const ThingId &inputThing, const StateTypeId &inputState, const ThingId &outputThing, const StateTypeId &outputState):
    m_id(id),
    m_inputThingId(inputThing),
    m_inputStateTypeId(inputState),
    m_outputThingId(outputThing),
    m_outputStateTypeId(outputState)
{

}

IOConnectionId IOConnection::id() const
{
    return m_id;
}

ThingId IOConnection::inputThingId() const
{
    return m_inputThingId;
}

StateTypeId IOConnection::inputStateTypeId() const
{
    return m_inputStateTypeId;
}

ThingId IOConnection::outputThingId() const
{
    return m_outputThingId;
}

StateTypeId IOConnection::outputStateTypeId() const
{
    return m_outputStateTypeId;
}

QVariant IOConnections::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void IOConnections::put(const QVariant &variant)
{
    append(variant.value<IOConnection>());
}