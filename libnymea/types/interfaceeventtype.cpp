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

bool InterfaceEventType::loggingOverride() const
{
    return m_loggingOverride;
}

void InterfaceEventType::setLoggingOverride(bool loggingOverride)
{
    m_loggingOverride = loggingOverride;
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
