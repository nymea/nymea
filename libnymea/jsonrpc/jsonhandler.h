#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QObject>
#include <QVariantMap>
#include <QMetaMethod>

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
    QVariantMap jsonObjects() const;
    QVariantMap jsonMethods() const;
    QVariantMap jsonNotifications() const;


    template<typename T> static QString enumRef();
    static QString objectRef(const QString &objectName);

    template<typename T> static QString enumValueName(T value);
    template<typename T> static T enumNameToValue(const QString &name);

    static BasicType variantTypeToBasicType(QVariant::Type variantType);
    static QVariant::Type basicTypeToVariantType(BasicType basicType);

protected:
    template <typename T> void registerEnum();
    void registerObject(const QString &name, const QVariantMap &object);
    void registerMethod(const QString &name, const QString &description, const QVariantMap &params, const QVariantMap &returns, bool deprecated = false);
    void registerNotification(const QString &name, const QString &description, const QVariantMap &params, bool deprecated = false);

    JsonReply *createReply(const QVariantMap &data) const;
    JsonReply *createAsyncReply(const QString &method) const;


private:
    QVariantMap m_enums;
    QVariantMap m_objects;
    QVariantMap m_methods;
    QVariantMap m_notifications;
};

template<typename T>
void JsonHandler::registerEnum()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    QStringList values;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        values << metaEnum.key(i);
    }
    m_enums.insert(metaEnum.name(), values);

}

template<typename T>
QString JsonHandler::enumRef()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    return QString("$ref:%1").arg(metaEnum.name());
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

#endif // JSONHANDLER_H
