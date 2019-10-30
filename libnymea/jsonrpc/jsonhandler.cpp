/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "jsonhandler.h"

#include "loggingcategories.h"

#include <QDebug>
#include <QDateTime>

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

QVariant JsonHandler::pack(const QMetaObject &metaObject, const void *value) const
{
    QString className = QString(metaObject.className()).split("::").last();
    if (m_listMetaObjects.contains(className)) {
        QVariantList ret;
        QMetaProperty countProperty = metaObject.property(metaObject.indexOfProperty("count"));
        QMetaObject entryMetaObject = m_metaObjects.value(m_listEntryTypes.value(metaObject.className()));
        int count = countProperty.readOnGadget(value).toInt();
        QMetaMethod getMethod = metaObject.method(metaObject.indexOfMethod("get(int)"));
        for (int i = 0; i < count; i++) {
            QVariant entry;
            getMethod.invokeOnGadget(const_cast<void*>(value), Q_RETURN_ARG(QVariant, entry), Q_ARG(int, i));
            ret.append(pack(entryMetaObject, entry.data()));
        }
        return ret;
    }

    if (m_metaObjects.contains(className)) {
        QVariantMap ret;
        for (int i = 0; i < metaObject.propertyCount(); i++) {
            QMetaProperty metaProperty = metaObject.property(i);

            // Skip QObject's objectName property
            if (metaProperty.name() == QStringLiteral("objectName")) {
                continue;
            }

            // Pack flags
            if (metaProperty.isFlagType()) {
                QString flagName = QString(metaProperty.typeName()).split("::").last();
                Q_ASSERT_X(m_metaFlags.contains(flagName), this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered in this handler.").arg(className).arg(flagName).toUtf8());
                QMetaEnum metaFlag = m_metaFlags.value(flagName);
                int flagValue = metaProperty.readOnGadget(value).toInt();
                QStringList flags;
                for (int i = 0; i < metaFlag.keyCount(); i++) {
                    if ((metaFlag.value(i) & flagValue) > 0) {
                        flags.append(metaFlag.key(i));
                    }
                }
                ret.insert(metaProperty.name(), flags);
                continue;
            }

            // Pack enums
            if (metaProperty.isEnumType()) {
                QString enumName = QString(metaProperty.typeName()).split("::").last();
                Q_ASSERT_X(m_metaEnums.contains(enumName), this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered in this handler.").arg(className).arg(metaProperty.typeName()).toUtf8());
                QMetaEnum metaEnum = m_metaEnums.value(enumName);
                ret.insert(metaProperty.name(), metaEnum.key(metaProperty.readOnGadget(value).toInt()));
                continue;
            }

            // Basic type/Variant type
            if (metaProperty.typeName() == QStringLiteral("QVariant::Type")) {
                QMetaEnum metaEnum = QMetaEnum::fromType<BasicType>();
                ret.insert(metaProperty.name(), metaEnum.key(variantTypeToBasicType(metaProperty.readOnGadget(value).template value<QVariant::Type>())));
                continue;
            }

            // Our own objects
            if (metaProperty.type() == QVariant::UserType) {
                if (m_listMetaObjects.contains(metaProperty.typeName())) {
                    QMetaObject entryMetaObject = m_listMetaObjects.value(metaProperty.typeName());
                    ret.insert(metaProperty.name(), pack(entryMetaObject, metaProperty.readOnGadget(value).data()));
                    continue;
                }

                if (m_metaObjects.contains(metaProperty.typeName())) {
                    QMetaObject entryMetaObject = m_metaObjects.value(metaProperty.typeName());
                    ret.insert(metaProperty.name(), pack(entryMetaObject, metaProperty.readOnGadget(value).data()));
                    continue;
                }

                Q_ASSERT_X(false, this->metaObject()->className(), QString("Unregistered property type: %1").arg(metaProperty.typeName()).toUtf8());
                qCWarning(dcJsonRpc()) << "Cannot pack property of unregistered object type" << metaProperty.typeName();
                continue;
            }

            // Standard properties, QString, int etc... If it's not optional, or if it's not empty, pack it up
            if (!metaProperty.isUser() || !metaProperty.readOnGadget(value).isNull()) {
                QVariant variant = metaProperty.readOnGadget(value);
                // Special treatment for QDateTime (converting to time_t)
                if (metaProperty.type() == QVariant::DateTime) {
                    variant = variant.toDateTime().toTime_t();
                }
                ret.insert(metaProperty.name(), variant);
            }
        }
        return ret;
    }

    Q_ASSERT_X(false, this->metaObject()->className(), QString("Unregistered object type: %1").arg(className).toUtf8());
    qCWarning(dcJsonRpc()) << "Cannot pack object of unregistered type" << className;
    return QVariant();
//    QVariantMap ret;
////    qWarning() << "+ Packing" << metaObject.className();
//    for (int i = 0; i < metaObject.propertyCount(); i++) {
//        QMetaProperty metaProperty = metaObject.property(i);
//        if (metaProperty.name() == QStringLiteral("objectName")) {
//            continue; // Skip QObject's objectName property
//        }

//        QVariant val = metaProperty.readOnGadget(value);
////        qWarning() << "|- Property:" << metaProperty.name() << metaProperty.readOnGadget(value) << metaProperty.type() << metaProperty.typeName();
////        qWarning() << "|-- All list types:" << m_listMetaObjects.keys();
//        if (metaProperty.type() == QVariant::UserType) {
//            if (metaProperty.typeName() == QStringLiteral("QVariant::Type")) {
//                QMetaEnum metaEnum = QMetaEnum::fromType<BasicType>();
////                qWarning() << "|--" << metaProperty.readOnGadget(value).toInt() << metaEnum.key(metaProperty.readOnGadget(value).toInt());
//                ret.insert(metaProperty.name(), metaEnum.key(variantTypeToBasicType(metaProperty.readOnGadget(value).template value<QVariant::Type>())));
//            } else if (m_listMetaObjects.contains(metaProperty.typeName())) {
//                QVariant listObject = metaProperty.readOnGadget(value);
//                QMetaObject listMetaObject = m_listMetaObjects.value(metaProperty.typeName());
//                QMetaProperty countProperty = listMetaObject.property(listMetaObject.indexOfProperty("count"));
//                int listCount = countProperty.readOnGadget(listObject.constData()).toInt();
////                qWarning() << "Packing list type" << listObject << "count is" << listCount;
//                QMetaMethod metaMethod = listMetaObject.method(listMetaObject.indexOfMethod("get(int)"));
////                qWarning() << "get method" << listMetaObject.indexOfMethod("get(int)") << listMetaObject.method(0).name() << QMetaObject::normalizedSignature("QVariant get(int)");

//                QMetaObject entryMetaObject = m_metaObjects.value(m_listEntryTypes.value(listMetaObject.className()));
//                QVariantList list;
//                for (int i = 0; i < listCount; i++) {
//                    QVariant entry;
//                    metaMethod.invokeOnGadget(listObject.data(), Q_RETURN_ARG(QVariant, entry), Q_ARG(int, i));
////                    qWarning() << "|---Feckin hell" << entry;

//                    list.append(pack(entryMetaObject, entry.data()));
//                }

//                ret.insert(metaProperty.name(), list);


//            } else {
//                Q_ASSERT_X(false, this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered in this handler.").arg(metaObject.className()).arg(metaProperty.typeName()).toUtf8());
//            }
//        } else if (metaProperty.isFlagType()) {
//            QMetaEnum metaFlag = m_metaFlags.value(QString(metaProperty.typeName()).split("::").last());
////            QMetaEnum metaEnum = m_metaEnums.value(m_flagsEnums.value(metaFlag.name()));
//            int flagValue = metaProperty.readOnGadget(value).toInt();
////            qWarning() << "|-- Flag" << flagValue << metaFlag.name() << metaFlag.keyCount() << metaProperty.type();
//            QStringList flags;
//            for (int i = 0; i < metaFlag.keyCount(); i++) {
////                qWarning() << "|--- flag key:" << metaFlag.key(i) << metaFlag.value(i);
//                if ((metaFlag.value(i) & flagValue) > 0) {
//                    flags.append(metaFlag.key(i));
//                }
//            }
//            ret.insert(metaProperty.name(), flags);
//        } else if (metaProperty.isEnumType()) {
//            QString enumName = QString(metaProperty.typeName()).split("::").last();
//            Q_ASSERT_X(m_metaEnums.contains(enumName), this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered int this handler.").arg(metaObject.className()).arg(metaProperty.typeName()).toUtf8());
//            QMetaEnum metaEnum = m_metaEnums.value(enumName);
////            qWarning() << "|-- Enum: Name:" <<  metaEnum.name() << "as int:" << metaEnum.key(metaProperty.readOnGadget(value).toInt()) << "All enums:" << m_metaEnums.keys();
//            ret.insert(metaProperty.name(), metaEnum.key(metaProperty.readOnGadget(value).toInt()));
//        } else if (!metaProperty.isUser() || !metaProperty.readOnGadget(value).isNull()) {
////            qWarning() << "|-- property" << metaProperty.name() << metaProperty.readOnGadget(value);
//            ret.insert(metaProperty.name(), metaProperty.readOnGadget(value));
//        }
//    }
//    return ret;

}

