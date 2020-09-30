#ifndef APIKEY_H
#define APIKEY_H

#include <QString>
#include <QHash>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcApiKeys)

class ApiKey
{
public:
    ApiKey();

    QByteArray data(const QString &key) const;
    void insert(const QString &key, const QByteArray &data);

private:
    QHash<QString, QByteArray> m_data;
};

#endif // APIKEY_H
