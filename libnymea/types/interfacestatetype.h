#ifndef INTERFACESTATETYPE_H
#define INTERFACESTATETYPE_H

#include "statetype.h"

class InterfaceStateType: public StateType
{
public:
    InterfaceStateType();

    bool optional() const;
    void setOptional(bool optional);

    bool logged() const;
    void setLogged(bool logged);

private:
    bool m_optional = false;
    bool m_logged = false;
};

class InterfaceStateTypes: public QList<InterfaceStateType>
{
public:
    InterfaceStateTypes() = default;
    InterfaceStateTypes(const QList<InterfaceStateType> &other);
    InterfaceStateType findByName(const QString &name);
};

#endif // INTERFACESTATETYPE_H
