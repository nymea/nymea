#ifndef INTERFACEEVENTTYPE_H
#define INTERFACEEVENTTYPE_H

#include "eventtype.h"

class InterfaceEventType: public EventType
{
public:
    InterfaceEventType();

    bool optional() const;
    void setOptional(bool optional);

private:
    bool m_optional = false;
};

class InterfaceEventTypes: public QList<InterfaceEventType>
{
public:
    InterfaceEventTypes() = default;
    InterfaceEventTypes(const QList<InterfaceEventType> &other);
    InterfaceEventType findByName(const QString &name);
};

#endif // INTERFACEEVENTTYPE_H
