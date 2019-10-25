#include "jsonhandler.h"

#include "loggingcategories.h"

#include <QDebug>

JsonHandler::JsonHandler(QObject *parent) : QObject(parent)
{
    registerEnum<BasicType>();
}

QVariantMap JsonHandler::jsonEnums() const
{
    return m_enums;
}

QVariantMap JsonHandler::jsonFlags() const
{
    return m_flags;
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
    case QVariant::DateTime:
        return Uint; // DateTime is represented as time_t
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

QVariantMap JsonHandler::pack(const QMetaObject &metaObject, const void *value) const
{
    QVariantMap ret;
//    qWarning() << "+ Packing" << metaObject.className();
    for (int i = 0; i < metaObject.propertyCount(); i++) {
        QMetaProperty metaProperty = metaObject.property(i);
        if (metaProperty.name() == QStringLiteral("objectName")) {
            continue; // Skip QObject's objectName property
        }

        QVariant val = metaProperty.readOnGadget(value);
//        qWarning() << "|- Property:" << metaProperty.name() << metaProperty.readOnGadget(value) << metaProperty.type() << metaProperty.typeName();
//        qWarning() << "|-- All list types:" << m_listMetaObjects.keys();
        if (metaProperty.type() == QVariant::UserType) {
            if (metaProperty.typeName() == QStringLiteral("QVariant::Type")) {
                QMetaEnum metaEnum = QMetaEnum::fromType<BasicType>();
//                qWarning() << "|--" << metaProperty.readOnGadget(value).toInt() << metaEnum.key(metaProperty.readOnGadget(value).toInt());
                ret.insert(metaProperty.name(), metaEnum.key(variantTypeToBasicType(metaProperty.readOnGadget(value).template value<QVariant::Type>())));
            } else if (m_listMetaObjects.contains(metaProperty.typeName())) {
                QVariant listObject = metaProperty.readOnGadget(value);
                QMetaObject listMetaObject = m_listMetaObjects.value(metaProperty.typeName());
                QMetaProperty countProperty = listMetaObject.property(listMetaObject.indexOfProperty("count"));
                int listCount = countProperty.readOnGadget(listObject.constData()).toInt();
//                qWarning() << "Packing list type" << listObject << "count is" << listCount;
                QMetaMethod metaMethod = listMetaObject.method(listMetaObject.indexOfMethod("get(int)"));
//                qWarning() << "get method" << listMetaObject.indexOfMethod("get(int)") << listMetaObject.method(0).name() << QMetaObject::normalizedSignature("QVariant get(int)");

                QMetaObject entryMetaObject = m_metaObjects.value(m_listEntryTypes.value(listMetaObject.className()));
                QVariantList list;
                for (int i = 0; i < listCount; i++) {
                    QVariant entry;
                    metaMethod.invokeOnGadget(listObject.data(), Q_RETURN_ARG(QVariant, entry), Q_ARG(int, i));
//                    qWarning() << "|---Feckin hell" << entry;

                    list.append(pack(entryMetaObject, entry.data()));
                }

                ret.insert(metaProperty.name(), list);


            } else {
                Q_ASSERT_X(false, this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered in this handler.").arg(metaObject.className()).arg(metaProperty.typeName()).toUtf8());
            }
        } else if (metaProperty.isFlagType()) {
            QMetaEnum metaFlag = m_metaFlags.value(QString(metaProperty.typeName()).split("::").last());
//            QMetaEnum metaEnum = m_metaEnums.value(m_flagsEnums.value(metaFlag.name()));
            int flagValue = metaProperty.readOnGadget(value).toInt();
//            qWarning() << "|-- Flag" << flagValue << metaFlag.name() << metaFlag.keyCount() << metaProperty.type();
            QStringList flags;
            for (int i = 0; i < metaFlag.keyCount(); i++) {
//                qWarning() << "|--- flag key:" << metaFlag.key(i) << metaFlag.value(i);
                if ((metaFlag.value(i) & flagValue) > 0) {
                    flags.append(metaFlag.key(i));
                }
            }
            ret.insert(metaProperty.name(), flags);
        } else if (metaProperty.isEnumType()) {
            QString enumName = QString(metaProperty.typeName()).split("::").last();
            Q_ASSERT_X(m_metaEnums.contains(enumName), this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered int this handler.").arg(metaObject.className()).arg(metaProperty.typeName()).toUtf8());
            QMetaEnum metaEnum = m_metaEnums.value(enumName);
//            qWarning() << "|-- Enum: Name:" <<  metaEnum.name() << "as int:" << metaEnum.key(metaProperty.readOnGadget(value).toInt()) << "All enums:" << m_metaEnums.keys();
            ret.insert(metaProperty.name(), metaEnum.key(metaProperty.readOnGadget(value).toInt()));
        } else if (!metaProperty.isUser() || !metaProperty.readOnGadget(value).isNull()) {
//            qWarning() << "|-- property" << metaProperty.name() << metaProperty.readOnGadget(value);
            ret.insert(metaProperty.name(), metaProperty.readOnGadget(value));
        }
    }
    return ret;
}

