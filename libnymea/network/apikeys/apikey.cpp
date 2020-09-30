#include "apikey.h"

#include "loggingcategories.h"
NYMEA_LOGGING_CATEGORY(dcApiKeys, "ApiKeys")

ApiKey::ApiKey()
{

}

/*!
 * \brief ApiKey::data
 * \param key
 * \return Retrns the data for key. For example data("key") or data("clientId")
 * An ApiKey can have multiple properties, like appid, clientsecret, scope information etc.
 */
QByteArray ApiKey::data(const QString &key) const
{
    return m_data.value(key);
}

/*!
 * \brief ApiKey::insert
 * Insert a key value pair in the this api key. For example insert("appid", "...").
 * An ApiKey can have multiple properties, like appid, clientsecret, scope information etc.
 * \param key
 * \param data
 */
void ApiKey::insert(const QString &key, const QByteArray &data)
{
    m_data.insert(key, data);
}
