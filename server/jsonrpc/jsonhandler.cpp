#include "jsonhandler.h"

#include <QMetaMethod>
#include <QDebug>

JsonHandler::JsonHandler(QObject *parent) :
    QObject(parent)
{
}

QVariantMap JsonHandler::introspect()
{
    QVariantMap data;
    for (int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.methodType() == QMetaMethod::Method) {
            QVariantMap methodData;
            if (!m_descriptions.contains(method.name()) || !m_params.contains(method.name()) || !m_returns.contains(method.name())) {
                continue;
            }
            methodData.insert("description", m_descriptions.value(method.name()));
            methodData.insert("params", m_params.value(method.name()));
            methodData.insert("returns", m_returns.value(method.name()));
            data.insert(name() + "." + method.name(), methodData);
        }
    }
    return data;
}

bool JsonHandler::hasMethod(const QString &methodName)
{
    return m_descriptions.contains(methodName) && m_params.contains(methodName) && m_returns.contains(methodName);
}

bool JsonHandler::validateParams(const QString &methodName, const QVariantMap &params)
{
    QVariantMap paramTemplate = m_params.value(methodName);
    return JsonTypes::validateMap(paramTemplate, params);
}

bool JsonHandler::validateReturns(const QString &methodName, const QVariantMap &returns)
{
    QVariantMap returnsTemplate = m_returns.value(methodName);
    return JsonTypes::validateMap(returnsTemplate, returns);
}

void JsonHandler::setDescription(const QString &methodName, const QString &description)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_descriptions.insert(methodName, description);
            return;
        }
    }
    qWarning() << "Cannot set description. No such method:" << methodName;
}

void JsonHandler::setParams(const QString &methodName, const QVariantMap &params)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_params.insert(methodName, params);
            return;
        }
    }
    qWarning() << "Cannot set params. No such method:" << methodName;
}

void JsonHandler::setReturns(const QString &methodName, const QVariantMap &returns)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_returns.insert(methodName, returns);
            return;
        }
    }
    qWarning() << "Cannot set returns. No such method:" << methodName;
}
