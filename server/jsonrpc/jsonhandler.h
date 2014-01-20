#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include "jsontypes.h"

#include <QObject>
#include <QVariantMap>

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = 0);

    virtual QString name() const = 0;

    QVariantMap introspect();

    bool hasMethod(const QString &methodName);
    bool validateParams(const QString &methodName, const QVariantMap &params);
    bool validateReturns(const QString &methodName, const QVariantMap &returns);

protected:
    void setDescription(const QString &methodName, const QString &description);
    void setParams(const QString &methodName, const QVariantMap &params);
    void setReturns(const QString &methodName, const QVariantMap &returns);

private:
    QHash<QString, QString> m_descriptions;
    QHash<QString, QVariantMap> m_params;
    QHash<QString, QVariantMap> m_returns;
};

#endif // JSONHANDLER_H
