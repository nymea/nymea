#include "script.h"

namespace nymeaserver {

Script::Script()
{

}

QUuid Script::id() const
{
    return m_id;
}

void Script::setId(const QUuid &id)
{
    m_id = id;
}

QString Script::name() const
{
    return m_name;
}

void Script::setName(const QString &name)
{
    m_name = name;
}

Scripts::Scripts()
{

}

Scripts::Scripts(const QList<Script> &other):
    QList<Script>(other)
{

}

QVariant Scripts::get(int index)
{
    return QVariant::fromValue(at(index));
}

void Scripts::put(const QVariant &value)
{
    append(value.value<Script>());
}

}
