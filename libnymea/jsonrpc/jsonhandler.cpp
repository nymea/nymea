/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "jsonhandler.h"
#include "typeutils.h"
#include "loggingcategories.h"

#include <QDebug>
#include <QDateTime>

JsonHandler::JsonHandler(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QMetaType::Type>();
    registerEnum<BasicType>();
}

QHash<QString, QString> JsonHandler::cacheHashes() const
{
    return QHash<QString, QString>();
}

QVariantMap JsonHandler::translateNotification(const QString &notification, const QVariantMap &params, const QLocale &locale)
{
    Q_UNUSED(notification)
    Q_UNUSED(locale)
    return params;
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

JsonHandler::BasicType JsonHandler::variantTypeToBasicType(QMetaType::Type variantType)
{
    switch (variantType) {
    case QMetaType::QUuid:
        return Uuid;
    case QMetaType::QString:
        return String;
    case QMetaType::QStringList:
        return StringList;
    case QMetaType::Int:
        return Int;
    case QMetaType::UInt:
        return Uint;
    case QMetaType::Double:
        return Double;
    case QMetaType::Bool:
        return Bool;
    case QMetaType::QColor:
        return Color;
    case QMetaType::QTime:
        return Time;
    case QMetaType::QVariantMap:
        return Object;
    case QMetaType::QDateTime:
        return Uint; // DateTime is represented as time_t
    default:
        return Variant;
    }
}

QMetaType::Type JsonHandler::basicTypeToVariantType(JsonHandler::BasicType basicType)
{
    switch (basicType) {
    case Uuid:
        return QMetaType::QUuid;
    case String:
        return QMetaType::QString;
    case StringList:
        return QMetaType::QStringList;
    case Int:
        return QMetaType::Int;
    case Uint:
        return QMetaType::UInt;
    case Double:
        return QMetaType::Double;
    case Bool:
        return QMetaType::Bool;
    case Color:
        return QMetaType::QColor;
    case Time:
        return QMetaType::QTime;
    case Object:
        return QMetaType::QVariantMap;
    case Variant:
        return QMetaType::UnknownType;
    }
    return QMetaType::UnknownType;
}

void JsonHandler::registerObject(const QString &name, const QVariantMap &object)
{
    m_objects.insert(name, object);
}

void JsonHandler::registerMethod(const QString &name, const QString &description, const QVariantMap &params, const QVariantMap &returns, Types::PermissionScope permissionScope, const QString &deprecationInfo)
{
    QVariantMap methodData;
    methodData.insert("description", description);
    methodData.insert("params", params);
    methodData.insert("returns", returns);
    methodData.insert("permissionScope", enumValueName(permissionScope));
    if (!deprecationInfo.isEmpty()) {
        methodData.insert("deprecated", deprecationInfo);
    }

    m_methods.insert(name, methodData);
}

void JsonHandler::registerNotification(const QString &name, const QString &description, const QVariantMap &params, const QString &deprecationInfo)
{
    QVariantMap notificationData;
    notificationData.insert("description", description);
    notificationData.insert("params", params);
    if (!deprecationInfo.isEmpty()) {
        notificationData.insert("deprecated", deprecationInfo);
    }

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

void JsonHandler::registerObject(const QMetaObject &metaObject)
{
    QString className = QString(metaObject.className()).split("::").last();
    QVariantMap description;
    for (int i = 0; i < metaObject.propertyCount(); i++) {
        QMetaProperty metaProperty = metaObject.property(i);
        QString name = metaProperty.name();
        if (name == "objectName") {
            continue; // Skip QObject's objectName property
        }
        if (metaProperty.isUser()) {
            name.prepend("o:");
        }
        if (!metaProperty.isWritable()) {
            name.prepend("r:");
        }
        if (metaProperty.revision() == 1) {
            name.prepend("d:");
        }
        QVariant typeName;
        if (metaProperty.typeId() == QMetaType::User) {
            if (metaProperty.typeName() == QStringLiteral("QMetaType::Type")) {
                typeName = QString("$ref:BasicType");
            } else if (QString(metaProperty.typeName()).startsWith("QList")) {
                QString elementType = QString(metaProperty.typeName()).remove("QList<").remove(">");
                if (elementType == "ThingId" || elementType == "EventTypeId" || elementType == "StateTypeId" || elementType == "ActionTypeId") {
                    elementType = "QUuid";
                }
                QMetaType::Type variantType = static_cast<QMetaType::Type>(QVariant::nameToType(elementType.toUtf8()));
                typeName = QVariantList() << enumValueName(variantTypeToBasicType(variantType));
            } else {
                typeName = QString("$ref:%1").arg(QString(metaProperty.typeName()).split("::").last());
            }
        } else if (metaProperty.isEnumType()) {
            typeName = QString("$ref:%1").arg(QString(metaProperty.typeName()).split("::").last());
        } else if (metaProperty.isFlagType()) {
            typeName = QVariantList() << "$ref:" + m_flagsEnums.value(metaProperty.name());
        } else if (metaProperty.typeId()== QMetaType::QVariantList) {
            typeName = QVariantList() << enumValueName(Variant);
        } else {
            typeName = enumValueName(variantTypeToBasicType(static_cast<QMetaType::Type>(metaProperty.type())));
        }
        description.insert(name, typeName);
    }
    m_objects.insert(className, description);
    m_metaObjects.insert(className, metaObject);
}

void JsonHandler::registerList(const QMetaObject &listMetaObject, const QMetaObject &metaObject)
{
    QString listTypeName = QString(listMetaObject.className()).split("::").last();
    QString objectTypeName = QString(metaObject.className()).split("::").last();
    m_objects.insert(listTypeName, QVariantList() << QVariant(QString("$ref:%1").arg(objectTypeName)));
    m_metaObjects.insert(listTypeName, listMetaObject);
    m_listMetaObjects.insert(listTypeName, listMetaObject);
    m_listEntryTypes.insert(listTypeName, objectTypeName);
    Q_ASSERT_X(listMetaObject.indexOfProperty("count") >= 0, "JsonHandler", QString("List type %1 does not implement \"count\" property!").arg(listTypeName).toUtf8());
    Q_ASSERT_X(listMetaObject.indexOfMethod("get(int)") >= 0, "JsonHandler", QString("List type %1 does not implement \"Q_INVOKABLE QVariant get(int index)\" method!").arg(listTypeName).toUtf8());
    Q_ASSERT_X(listMetaObject.indexOfMethod("put(QVariant)") >= 0, "JsonHandler", QString("List type %1 does not implement \"Q_INVOKABLE void put(QVariant variant)\" method!").arg(listTypeName).toUtf8());
}

void JsonHandler::registerObject(const QMetaObject &metaObject, const QMetaObject &listMetaObject)
{
    registerObject(metaObject);
    registerList(listMetaObject, metaObject);
}

QVariant JsonHandler::pack(const QMetaObject &metaObject, const void *value) const
{
    QString className = QString(metaObject.className()).split("::").last();
    if (m_listMetaObjects.contains(className)) {
        QVariantList ret;
        QMetaProperty countProperty = metaObject.property(metaObject.indexOfProperty("count"));
        QMetaObject entryMetaObject = m_metaObjects.value(m_listEntryTypes.value(className));
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

            QVariant propertyValue = metaProperty.readOnGadget(value);
            // If it's optional and empty, we may skip it
            if (metaProperty.isUser() && (!propertyValue.isValid() || propertyValue.isNull())) {
                continue;
            }

            // Pack flags
            if (metaProperty.isFlagType()) {
                QString flagName = QString(metaProperty.typeName()).split("::").last();
                Q_ASSERT_X(m_metaFlags.contains(flagName), this->metaObject()->className(), QString("Cannot pack %1. %2 is not registered in this handler.").arg(className).arg(flagName).toUtf8());
                QMetaEnum metaFlag = m_metaFlags.value(flagName);
                int flagValue = propertyValue.toInt();
                QStringList flags;
                for (int i = 0; i < metaFlag.keyCount(); i++) {
                    int flag = metaFlag.value(i) & flagValue;
                    if (flag == metaFlag.value(i) && flag > 0) {
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
                ret.insert(metaProperty.name(), metaEnum.key(propertyValue.toInt()));
                continue;
            }

            // Basic type/Variant type
            if (metaProperty.typeName() == QStringLiteral("QMetaType::Type")) {
                QMetaEnum metaEnum = QMetaEnum::fromType<BasicType>();
                ret.insert(metaProperty.name(), metaEnum.key(variantTypeToBasicType(propertyValue.template value<QMetaType::Type>())));
                continue;
            }

            // Our own objects
            if (metaProperty.typeId() == QMetaType::User) {
                QString propertyTypeName = QString(metaProperty.typeName()).split("::").last();
                if (m_listMetaObjects.contains(propertyTypeName)) {
                    QMetaObject entryMetaObject = m_listMetaObjects.value(propertyTypeName);
                    QVariant packed = pack(entryMetaObject, propertyValue.data());
                    if (!metaProperty.isUser() || packed.toList().count() > 0) {
                        ret.insert(metaProperty.name(), packed);
                    }
                    continue;
                }

                if (m_metaObjects.contains(propertyTypeName)) {
                    QMetaObject entryMetaObject = m_metaObjects.value(propertyTypeName);
                    QVariant packed = pack(entryMetaObject, propertyValue.data());
                    int isValidIndex = entryMetaObject.indexOfMethod("isValid()");
                    bool isValid = true;
                    if (isValidIndex >= 0) {
                        QMetaMethod isValidMethod = entryMetaObject.method(isValidIndex);
                        isValidMethod.invokeOnGadget(propertyValue.data(), Q_RETURN_ARG(bool, isValid));
                    }
                    if (isValid || !metaProperty.isUser()) {
                        ret.insert(metaProperty.name(), packed);
                    }
                    continue;
                }

                // Manually converting QList<BasicType>... Only QVariantList is known to the meta system
                if (propertyTypeName.startsWith("QList<")) {
                    QVariantList list;
                    if (propertyTypeName == "QList<int>") {
                        foreach (int entry, propertyValue.value<QList<int>>()) {
                            list << entry;
                        }
                    } else if (propertyTypeName == "QList<QUuid>") {
                        foreach (const QUuid &entry, propertyValue.value<QList<QUuid>>()) {
                            list << entry;
                        }
                    } else if (propertyTypeName == "QList<ThingId>") {
                        foreach (const ThingId &entry, propertyValue.value<QList<ThingId>>()) {
                            list << entry;
                        }
                    } else if (propertyTypeName == "QList<EventTypeId>") {
                        foreach (const EventTypeId &entry, propertyValue.value<QList<EventTypeId>>()) {
                            list << entry;
                        }
                    } else if (propertyTypeName == "QList<StateTypeId>") {
                        foreach (const EventTypeId &entry, propertyValue.value<QList<StateTypeId>>()) {
                            list << entry;
                        }
                    } else if (propertyTypeName == "QList<ActionTypeId>") {
                        foreach (const EventTypeId &entry, propertyValue.value<QList<ActionTypeId>>()) {
                            list << entry;
                        }
                    } else if (propertyTypeName == "QList<QDateTime>") {
                        foreach (const QDateTime &timestamp, propertyValue.value<QList<QDateTime>>()) {
                            list << timestamp.toMSecsSinceEpoch() / 1000;
                        }
                    } else {
                        Q_ASSERT_X(false, this->metaObject()->className(), QString("Unhandled list type: %1").arg(propertyTypeName).toUtf8());
                        qCWarning(dcJsonRpc()) << "Cannot pack property of unhandled list type" << propertyTypeName;
                    }

                    if (!list.isEmpty() || !metaProperty.isUser()) {
                        ret.insert(metaProperty.name(), list);
                    }
                    continue;
                }

                Q_ASSERT_X(false, this->metaObject()->className(), QString("Unregistered property type: %1").arg(propertyTypeName).toUtf8());
                qCWarning(dcJsonRpc()) << "Cannot pack property of unregistered object type" << propertyTypeName;
                continue;
            }

            // Standard properties, QString, int etc...
            // Special treatment for QDateTime (converting to time_t)
            if (metaProperty.typeId() == QMetaType::QDateTime) {
                QDateTime dateTime = propertyValue.toDateTime();
                if (metaProperty.isUser() && dateTime.toSecsSinceEpoch() == 0) {
                    continue;
                }
                propertyValue = propertyValue.toDateTime().toSecsSinceEpoch();
            } else if (metaProperty.typeId() == QMetaType::QTime) {
                propertyValue = propertyValue.toTime().toString("hh:mm");
            }
            ret.insert(metaProperty.name(), propertyValue);
        }
        return ret;
    }

    Q_ASSERT_X(false, this->metaObject()->className(), QString("Unregistered object type: %1").arg(className).toUtf8());
    qCWarning(dcJsonRpc()) << "Cannot pack object of unregistered type" << className;
    return QVariant();    
}

QVariant JsonHandler::unpack(const QMetaObject &metaObject, const QVariant &value) const
{
    QString typeName = QString(metaObject.className()).split("::").last();

    // If it's a list object, loop over count
    if (m_listMetaObjects.contains(typeName)) {
        if (static_cast<QMetaType::Type>(value.type()) != QMetaType::QVariantList) {
//            qCWarning(dcJsonRpc()) << "Cannot unpack" << typeName << ". Value is not in list format:" << value;
            return QVariant();
        }

        QVariantList list = value.toList();

        int typeId = QMetaType::type(metaObject.className());
        void* ptr = QMetaType::create(typeId);
        Q_ASSERT_X(typeId != 0, this->metaObject()->className(), QString("Cannot handle unregistered meta type %1").arg(metaObject.className()).toUtf8());

        QMetaObject entryMetaObject = m_metaObjects.value(m_listEntryTypes.value(typeName));
        QMetaMethod putMethod = metaObject.method(metaObject.indexOfMethod("put(QVariant)"));

        foreach (const QVariant &variant, list) {
            QVariant value = unpack(entryMetaObject, variant);
            putMethod.invokeOnGadget(ptr, Q_ARG(QVariant, value));
        }

        QVariant ret = QVariant(static_cast<QMetaType>(typeId), ptr);
        QMetaType::destroy(typeId, ptr);
        return ret;
    }

    // if it's an object, loop over all properties
    if (m_metaObjects.contains(typeName)) {
        QVariantMap map = value.toMap();
        int typeId = QMetaType::type(metaObject.className());
        QMetaType metaType = static_cast<QMetaType>(typeId);
        Q_ASSERT_X(typeId != 0, this->metaObject()->className(), QString("Cannot handle unregistered meta type %1").arg(typeName).toUtf8());
        void* ptr = QMetaType::create(typeId);
        for (int i = 0; i < metaObject.propertyCount(); i++) {
            QMetaProperty metaProperty = metaObject.property(i);
            if (metaProperty.name() == QStringLiteral("objectName")) {
                continue;
            }
            if (!metaProperty.isWritable()) {
                continue;
            }
            if (!metaProperty.isUser()) {
                Q_ASSERT_X(map.contains(metaProperty.name()), this->metaObject()->className(), QString("Missing property %1 in map.").arg(metaProperty.name()).toUtf8());
            }

            if (map.contains(metaProperty.name())) {

                QString propertyTypeName = QString(metaProperty.typeName()).split("::").last();
                QVariant variant = map.value(metaProperty.name());

                // recurse into child lists
                if (m_listMetaObjects.contains(propertyTypeName)) {
                    QMetaObject propertyMetaObject = m_listMetaObjects.value(propertyTypeName);
                    metaProperty.writeOnGadget(ptr, unpack(propertyMetaObject, variant));
                    continue;
                }

                // recurse into child objects
                if (m_metaObjects.contains(propertyTypeName)) {
                    QMetaObject propertyMetaObject = m_metaObjects.value(propertyTypeName);
                    metaProperty.writeOnGadget(ptr, unpack(propertyMetaObject, variant));
                    continue;
                }

                if (QString(metaProperty.typeName()).startsWith("QList<")) {
                    if (metaProperty.typeName() == QStringLiteral("QList<int>")) {
                        QList<int> intList;
                        foreach (const QVariant &val, variant.toList()) {
                            intList.append(val.toInt());
                        }
                        metaProperty.writeOnGadget(ptr, QVariant::fromValue(intList));
                    } else if (metaProperty.typeName() == QStringLiteral("QList<QUuid>")
                               || metaProperty.typeName() == QStringLiteral("QList<ThingId>")
                               || metaProperty.typeName() == QStringLiteral("QList<EventTypeId>")
                               || metaProperty.typeName() == QStringLiteral("QList<StateTypeId>")
                               || metaProperty.typeName() == QStringLiteral("QList<ActionTypeId>")) {
                        QList<QUuid> uuidList;
                        foreach (const QVariant &val, variant.toList()) {
                            uuidList.append(val.toUuid());
                        }
                        metaProperty.writeOnGadget(ptr, QVariant::fromValue(uuidList));
                    }
                    continue;
                }

                // Special treatment for QDateTime (convert from time_t)
                if (metaProperty.typeId() == QMetaType::QDateTime) {
                    variant = QDateTime::fromSecsSinceEpoch(variant.toUInt());
                } else if (metaProperty.typeId() == QMetaType::QTime) {
                    variant = QTime::fromString(variant.toString(), "hh:mm");
                }

                // For basic properties just write the veriant as is
                metaProperty.writeOnGadget(ptr, variant);
            }

        }
        QVariant ret = QVariant(metaType, ptr);
        QMetaType::destroy(typeId, ptr);
        return ret;
    }

    return QVariant();
}
