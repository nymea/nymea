#ifndef RULESHANDLER_H
#define RULESHANDLER_H

#include "jsonhandler.h"

class RulesHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit RulesHandler(QObject *parent = 0);

    QString name() const override;

    Q_INVOKABLE QVariantMap GetRules(const QVariantMap &params);

    Q_INVOKABLE QVariantMap AddRule(const QVariantMap &params);

};

#endif // RULESHANDLER_H
