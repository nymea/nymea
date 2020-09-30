#include "apikeystorage.h"

ApiKeyStorage::ApiKeyStorage(QObject *parent):
    QObject(parent)
{

}

ApiKey ApiKeyStorage::requestKey(const QString &name) const
{
    if (!m_keys.contains(name)) {
        qCWarning(dcApiKeys) << "API key not found for" << name;
    }
    return m_keys.value(name);
}

void ApiKeyStorage::insertKey(const QString &name, const ApiKey &key)
{
    if (m_keys.contains(name)) {
        m_keys[name] = key;
        emit keyUpdated(name, key);
    } else {
        m_keys.insert(name, key);
        emit keyAdded(name, key);
    }
}
