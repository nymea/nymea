#ifndef APIKEYSPROVIDERSLOADER_H
#define APIKEYSPROVIDERSLOADER_H

#include "network/apikeys/apikeysprovider.h"

class ApiKeysProvidersLoader: public QObject
{
    Q_OBJECT

public:
    ApiKeysProvidersLoader(QObject *parent = nullptr);

    QList<ApiKeysProvider*> providers() const;

    QHash<QString, ApiKey> allApiKeys() const;

private:
    QStringList pluginSearchDirs() const;
    void loadPlugin(const QString &file);

    QList<ApiKeysProvider*> m_providers;
};

#endif // APIKEYSPROVIDERSLOADER_H
