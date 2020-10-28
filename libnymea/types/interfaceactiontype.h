#ifndef INTERFACEACTIONTYPE_H
#define INTERFACEACTIONTYPE_H

#include "actiontype.h"

class InterfaceActionType: public ActionType
{
public:
    InterfaceActionType();

    bool optional() const;
    void setOptional(bool optional);

private:
    bool m_optional = false;
};

class InterfaceActionTypes: public QList<InterfaceActionType>
{
public:
    InterfaceActionTypes() = default;
    InterfaceActionTypes(const QList<InterfaceActionType> &other);
    InterfaceActionType findByName(const QString &name);
};

#endif // INTERFACEACTIONTYPE_H
