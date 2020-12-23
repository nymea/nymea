#ifndef INTERFACEEVENTTYPE_H
#define INTERFACEEVENTTYPE_H

#include "eventtype.h"

class InterfaceEventType: public EventType
{
public:
    InterfaceEventType();

    bool optional() const;
    void setOptional(bool optional);

    bool loggingOverride() const;
    void setLoggingOverride(bool loggingOverride);

private:
    bool m_optional = false;
    bool m_loggingOverride = false;
};

class InterfaceEventTypes: public QList<InterfaceEventType>
{
public:
    InterfaceEventTypes() = default;
    InterfaceEventTypes(const QList<InterfaceEventType> &other);
    InterfaceEventType findByName(const QString &name);
};

#endif // INTERFACEEVENTTYPE_H
