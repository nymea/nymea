#include "interfaceactiontype.h"

InterfaceActionType::InterfaceActionType()
{

}

bool InterfaceActionType::optional() const
{
    return m_optional;
}

void InterfaceActionType::setOptional(bool optional)
{
    m_optional = optional;
}

InterfaceActionTypes::InterfaceActionTypes(const QList<InterfaceActionType> &other):
    QList<InterfaceActionType>(other)
{

}

InterfaceActionType InterfaceActionTypes::findByName(const QString &name)
{
    foreach (const InterfaceActionType &iat, *this) {
        if (iat.name() == name) {
            return iat;
        }
    }
    return InterfaceActionType();
}
