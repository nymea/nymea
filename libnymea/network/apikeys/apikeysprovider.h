#ifndef APIKEYSPROVIDER_H
#define APIKEYSPROVIDER_H

#include "apikey.h"

#include <QObject>

class ApiKeysProvider: public QObject
{
    Q_OBJECT

public:
    ApiKeysProvider(QObject *parent = nullptr);
    virtual ~ApiKeysProvider() = default;

    virtual QHash<QString, ApiKey> apiKeys() const = 0;
};

Q_DECLARE_INTERFACE(ApiKeysProvider, "io.nymea.ApiKeysProvider")

#endif // APIKEYSPROVIDER_H
