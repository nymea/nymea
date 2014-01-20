#ifndef ACTIONHANDLER_H
#define ACTIONHANDLER_H

#include "jsonhandler.h"

class ActionHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ActionHandler(QObject *parent = 0);

    QString name() const;

    Q_INVOKABLE QVariantMap ExecuteAction(const QVariantMap &params);
};

#endif // ACTIONHANDLER_H
