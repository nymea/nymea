#include "interfacestatetype.h"

InterfaceStateType::InterfaceStateType()
{

}

bool InterfaceStateType::optional() const
{
    return m_optional;
}

void InterfaceStateType::setOptional(bool optional)
{
    m_optional = optional;
}

bool InterfaceStateType::logged() const
{
    return m_logged;
}

void InterfaceStateType::setLogged(bool logged)
{
    m_logged = logged;
}

InterfaceStateTypes::InterfaceStateTypes(const QList<InterfaceStateType> &other):
    QList<InterfaceStateType>(other)
{

}

InterfaceStateType InterfaceStateTypes::findByName(const QString &name)
{
    foreach (const InterfaceStateType &ist, *this) {
        if (ist.name() == name) {
            return ist;
        }
    }
    return InterfaceStateType();
}
