#ifndef APPDATAHANDLER_H
#define APPDATAHANDLER_H

#include <QObject>
#include "jsonrpc/jsonhandler.h"

class AppDataHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit AppDataHandler(QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *Store(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Load(const QVariantMap &params);

signals:

};

#endif // APPDATAHANDLER_H
