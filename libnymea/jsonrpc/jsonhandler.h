#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QObject>
#include <QVariantMap>
#include <QMetaMethod>
#include <QDebug>
#include <QVariant>
#include <QDateTime>

#include "jsonreply.h"

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    enum BasicType {
        Uuid,
        String,
        StringList,
        Int,
        Uint,
        Double,
        Bool,
        Variant,
        Color,
        Time,
        Object
    };
    Q_ENUM(BasicType)

    explicit JsonHandler(QObject *parent = nullptr);
    virtual ~JsonHandler() = default;

    virtual QString name() const = 0;

    QVariantMap jsonEnums() const;
    QVariantMap jsonFlags() const;
    QVariantMap jsonObjects() const;
    QVariantMap jsonMethods() const;
    QVariantMap jsonNotifications() const;


    template<typename T> static QString enumRef();
    template<typename T> static QString objectRef();
    static QString objectRef(const QString &objectName);

    template<typename T> static QString enumValueName(T value);
    template<typename T> static T enumNameToValue(const QString &name);

    static BasicType variantTypeToBasicType(QVariant::Type variantType);
    static QVariant::Type basicTypeToVariantType(BasicType basicType);

    template<typename T> QVariant pack(const T &value) const;
    template <typename T> T unpack(const QVariantMap &map) const;

protected:
    template <typename Enum> void registerEnum();
    template <typename Enum, typename Flags> void registerEnum();
    template <typename ObjectType> void registerObject();
    template <typename ObjectType, typename ListType> void registerObject();
    void registerObject(const QString &name, const QVariantMap &object);
    void registerMethod(const QString &name, const QString &description, const QVariantMap &params, const QVariantMap &returns, bool deprecated = false);
    void registerNotification(const QString &name, const QString &description, const QVariantMap &params, bool deprecated = false);

    JsonReply *createReply(const QVariantMap &data) const;
    JsonReply *createAsyncReply(const QString &method) const;

private:
    QVariant pack(const QMetaObject &metaObject, const void *gadget) const;

private:
    QVariantMap m_enums;
    QHash<QString, QMetaEnum> m_metaEnums;
    QVariantMap m_flags;
    QHash<QString, QMetaEnum> m_metaFlags;
    QHash<QString, QString> m_flagsEnums;
    QVariantMap m_objects;
    QHash<QString, QMetaObject> m_metaObjects;
    QHash<QString, QMetaObject> m_listMetaObjects;
    QHash<QString, QString> m_listEntryTypes;
    QVariantMap m_methods;
    QVariantMap m_notifications;
};
Q_DECLARE_METATYPE(QVariant::Type)

template<typename T>
void JsonHandler::registerEnum()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    QStringList values;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        values << metaEnum.key(i);
    }
    m_enums.insert(metaEnum.name(), values);
    m_metaEnums.insert(metaEnum.name(), metaEnum);
}

template<typename Enum, typename Flags>
void JsonHandler::registerEnum()
{
    registerEnum<Enum>();
    QMetaEnum metaEnum = QMetaEnum::fromType<Enum>();
    QMetaEnum metaFlags = QMetaEnum::fromType<Flags>();
    m_metaFlags.insert(metaFlags.name(), metaFlags);
    m_flagsEnums.insert(metaFlags.name(), metaEnum.name());
    m_flags.insert(metaFlags.name(), QVariantList() << QString("$ref:%1").arg(metaEnum.name()));
}

template<typename ObjectType>
void JsonHandler::registerObject()
{
    qRegisterMetaType<QVariant::Type>();
    QMetaObject metaObject = ObjectType::staticMetaObject;
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
        QVariant typeName;
//        qWarning() << ".-.-.-.-.-" << metaProperty.name() << metaProperty.type() << metaProperty.typeName();
        if (metaProperty.type() == QVariant::UserType) {
            if (metaProperty.typeName() == QStringLiteral("QVariant::Type")) {
                typeName = QString("$ref:BasicType");
            } else if (QString(metaProperty.typeName()).startsWith("QList")) {
                QString elementType = QString(metaProperty.typeName()).remove("QList<").remove(">");
                QVariant::Type variantType = QVariant::nameToType(elementType.toUtf8());
                typeName = QVariantList() << enumValueName(variantTypeToBasicType(variantType));
            } else {
                typeName = QString("$ref:%1").arg(QString(metaProperty.typeName()).split("::").last());
            }
        } else if (metaProperty.isEnumType()) {
            typeName = QString("$ref:%1").arg(QString(metaProperty.typeName()).split("::").last());
        } else if (metaProperty.isFlagType()) {
            typeName = QVariantList() << "$ref:" + m_flagsEnums.value(metaProperty.name());
        } else if (metaProperty.type() == QVariant::List) {
            typeName = QVariantList() << enumValueName(Variant);
        } else {
            typeName = enumValueName(variantTypeToBasicType(metaProperty.type()));
        }
        description.insert(name, typeName);
    }
    m_objects.insert(className, description);
    m_metaObjects.insert(className, metaObject);
}

template<typename ObjectType, typename ListType>
void JsonHandler::registerObject()
{
    registerObject<ObjectType>();
    QMetaObject metaObject = ObjectType::staticMetaObject;
    QMetaObject listMetaObject = ListType::staticMetaObject;
    QString listTypeName = QString(listMetaObject.className()).split("::").last();
    QString objectTypeName = QString(metaObject.className()).split("::").last();
    m_objects.insert(listTypeName, QVariantList() << QVariant(QString("$ref:%1").arg(objectTypeName)));
    m_metaObjects.insert(listTypeName, listMetaObject);
    m_listMetaObjects.insert(listTypeName, listMetaObject);
    m_listEntryTypes.insert(listTypeName, objectTypeName);
    Q_ASSERT_X(listMetaObject.indexOfProperty("count") >= 0, "JsonHandler", QString("List type %1 does not implement \"count\" property!").arg(listTypeName).toUtf8());
    Q_ASSERT_X(listMetaObject.indexOfMethod("get(int)") >= 0, "JsonHandler", QString("List type %1 does not implement \"Q_INVOKABLE QVariant get(int index)\" method!").arg(listTypeName).toUtf8());
}

template<typename T>
QString JsonHandler::enumRef()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    return QString("$ref:%1").arg(metaEnum.name());
}

template<typename T>
QString JsonHandler::objectRef()
{
    QMetaObject metaObject = T::staticMetaObject;
    return QString("$ref:%1").arg(QString(metaObject.className()).split("::").last());
}

template<typename T>
QString JsonHandler::enumValueName(T value)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    return metaEnum.valueToKey(value);
}

template<typename T>
T JsonHandler::enumNameToValue(const QString &name)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    return static_cast<T>(metaEnum.keyToValue(name.toUtf8()));
}

template<typename T>
QVariant JsonHandler::pack(const T &value) const
{
    QMetaObject metaObject = T::staticMetaObject;
    return pack(metaObject, static_cast<const void*>(&value));
}

template<typename T>
T JsonHandler::unpack(const QVariantMap &map) const
{
    T ret;
    QMetaObject metaObject = T::staticMetaObject;
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
            // Special treatment for QDateTime (convert from time_t)
            QVariant variant = map.value(metaProperty.name());
            if (metaProperty.type() == QVariant::DateTime) {
                variant = QDateTime::fromTime_t(variant.toUInt());
            }
            metaProperty.writeOnGadget(&ret, variant);
        }
    }
    return ret;
}



#endif // JSONHANDLER_H
