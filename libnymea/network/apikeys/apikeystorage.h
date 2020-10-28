#ifndef APIKEYSTORAGE_H
#define APIKEYSTORAGE_H

#include "apikey.h"

#include <QObject>

class ApiKeyStorage: public QObject
{
    Q_OBJECT
public:
    ApiKeyStorage(QObject *parent = nullptr);

    ApiKey requestKey(const QString &name) const;
    void insertKey(const QString &name, const ApiKey &key);

signals:
    void keyAdded(const QString &name, const ApiKey &key);
    void keyUpdated(const QString &name, const ApiKey &key);

private:
    QHash<QString, ApiKey> m_keys;
};

#endif // APIKEYSTORAGE_H
