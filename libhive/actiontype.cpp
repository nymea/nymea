/*!
    \class ActionType
    \brief Describes an \l{Action} for a \l{Device}.

    \ingroup types

    \sa Action
*/

#include "actiontype.h"

ActionType::ActionType(const QUuid &id):
    m_id(id)
{
}

QUuid ActionType::id() const
{
    return m_id;
}

QString ActionType::name() const
{
    return m_name;
}

void ActionType::setName(const QString &name)
{
    m_name = name;
}

QVariantList ActionType::parameters() const
{
    return m_parameters;
}

void ActionType::setParameters(const QVariantList &parameters)
{
    m_parameters = parameters;
}
