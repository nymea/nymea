#include "jsonhandler.h"

#include "loggingcategories.h"

#include <QDebug>

JsonHandler::JsonHandler(QObject *parent) : QObject(parent)
{
}

QVariantMap JsonHandler::jsonEnums() const
{
    return m_enums;
}

QVariantMap JsonHandler::jsonObjects() const
{
    return m_objects;
}

QVariantMap JsonHandler::jsonMethods() const
{
    return m_methods;
}

QVariantMap JsonHandler::jsonNotifications() const
{
    return m_notifications;
}

//QString JsonHandler::basicTypeName(JsonHandler::BasicType type)
//{
//    QMetaEnum metaEnum = QMetaEnum::fromType<BasicType>();
//    return metaEnum.valueToKey(type);
//}

QString JsonHandler::objectRef(const QString &objectName)
{
    return "$ref:" + objectName;
}

JsonHandler::BasicType JsonHandler::variantTypeToBasicType(QVariant::Type variantType)
{
    switch (variantType) {
    case QVariant::Uuid:
        return Uuid;
    case QVariant::String:
        return String;
    case QVariant::StringList:
        return StringList;
    case QVariant::Int:
        return Int;
    case QVariant::UInt:
        return Uint;
    case QVariant::Double:
        return Double;
    case QVariant::Bool:
        return Bool;
    case QVariant::Color:
        return Color;
    case QVariant::Time:
        return Time;
    case QVariant::Map:
        return Object;
    default:
        return Variant;
    }
}

QVariant::Type JsonHandler::basicTypeToVariantType(JsonHandler::BasicType basicType)
{
    switch (basicType) {
    case Uuid:
        return QVariant::Uuid;
    case String:
        return QVariant::String;
    case StringList:
        return QVariant::StringList;
    case Int:
        return QVariant::Int;
    case Uint:
        return QVariant::UInt;
    case Double:
        return QVariant::Double;
    case Bool:
        return QVariant::Bool;
    case Color:
        return QVariant::Color;
    case Time:
        return QVariant::Time;
    case Object:
        return QVariant::Map;
    case Variant:
        return QVariant::Invalid;
    }
    return QVariant::Invalid;
}

void JsonHandler::registerObject(const QString &name, const QVariantMap &object)
{
    m_objects.insert(name, object);
}

void JsonHandler::registerMethod(const QString &name, const QString &description, const QVariantMap &params, const QVariantMap &returns, bool /*deprecated*/)
{
    QVariantMap methodData;
    methodData.insert("description", description);
    methodData.insert("params", params);
    methodData.insert("returns", returns);
//    methodData.insert("deprecated", deprecated);

    m_methods.insert(name, methodData);
}

void JsonHandler::registerNotification(const QString &name, const QString &description, const QVariantMap &params, bool /*deprecated*/)
{
    QVariantMap notificationData;
    notificationData.insert("description", description);
    notificationData.insert("params", params);
//    notificationData.insert("deprecated", deprecated);

    m_notifications.insert(name, notificationData);
}

JsonReply *JsonHandler::createReply(const QVariantMap &data) const
{
    return JsonReply::createReply(const_cast<JsonHandler*>(this), data);
}

JsonReply *JsonHandler::createAsyncReply(const QString &method) const
{
    return JsonReply::createAsyncReply(const_cast<JsonHandler*>(this), method);
}

