#include "interfaceeventtype.h"

InterfaceEventType::InterfaceEventType()
{

}

bool InterfaceEventType::optional() const
{
    return m_optional;
}

void InterfaceEventType::setOptional(bool optional)
{
    m_optional = optional;
}

bool InterfaceEventType::logged() const
{
    return m_logged;
}

void InterfaceEventType::setLogged(bool logged)
{
    m_logged = logged;
}

InterfaceEventTypes::InterfaceEventTypes(const QList<InterfaceEventType> &other):
    QList<InterfaceEventType>(other)
{

}

InterfaceEventType InterfaceEventTypes::findByName(const QString &name)
{
    foreach (const InterfaceEventType &iet, *this) {
        if (iet.name() == name) {
            return iet;
        }
    }
    return InterfaceEventType();
}
