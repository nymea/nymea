#include "appdatahandler.h"
#include "jsonrpc/jsonrpcserver.h"

#include "nymeasettings.h"

#include <QSettings>
#include <QDir>

AppDataHandler::AppDataHandler(QObject *parent) : JsonHandler(parent)
{
    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Store an app data entry to the server. App data can be used by the client application "
                  "to store configuration values. The app data storage is a key-value pair storage. Each "
                  "entry value is identified by an appId, a key and optionally a group. The value data is "
                  "a bytearray and can contain arbitrary data, such as a JSON map or image data, however, "
                  "be aware of the maximum packet size for the used transport.\n"
                  "This might be useful to a client application to sync settings across multiple instances of "
                  "the same application.\n"
                  "The group parameter might be used to create groups for this application.\n"
                  "IMPORTANT: Currently no verification of the appId is done. The appid is merely a mechanism "
                  "to prevent different different client apps from colliding by using the same key for data "
                  "entries. This implies that the app data storage may not be suited for sensitive data given "
                  "that anyone with a valid server token can read it.\n ";
    params.insert("appId", enumValueName(String));
    params.insert("o:group", enumValueName(String));
    params.insert("key", enumValueName(String));
    params.insert("value", enumValueName(String));
    registerMethod("Store", description, params, returns, Types::PermissionScopeConfigureThings);

    description.clear(); params.clear(); returns.clear();
    description = "Retrieve an app data storage value that has previously been set with Store(). If no value "
                  "had been set for this appId/key combination before, an empty value will be returned.";
    params.insert("appId", enumValueName(String));
    params.insert("o:group", enumValueName(String));
    params.insert("key", enumValueName(String));
    returns.insert("value", enumValueName(String));
    registerMethod("Load", description, params, returns, Types::PermissionScopeControlThings);

    // Notifications
    description.clear(); params.clear();
    description = "Emitted whenever the app data is changed on the server.";
    params.insert("appId", enumValueName(String));
    params.insert("o:group", enumValueName(String));
    params.insert("key", enumValueName(String));
    params.insert("value", enumValueName(String));
    registerNotification("Changed", description, params);
}

QString AppDataHandler::name() const
{
    return "AppData";
}

JsonReply* AppDataHandler::Store(const QVariantMap &params)
{
    QString appId = params.value("appId").toString();
    QString group = params.value("group").toString();
    QString key = params.value("key").toString();
    QVariant value = params.value("value");

    // Note: we're using a different file for each group as QSettings tends to get slow with loads of keys.
    // Might be replaced with a DB at some point if needed. However, current estimate is this won't be
    // used for excessive amounts of data as it is mostly meant as a config file syncing mechanism.
    QSettings settings(NymeaSettings::storagePath() + "/appdata/" + appId + '/' + group + ".conf", QSettings::IniFormat);
    settings.setValue(key, value);

    QVariantMap notification;
    notification.insert("appId", appId);
    if (!group.isEmpty()) {
        notification.insert("group", group);
    }
    notification.insert("key", key);
    notification.insert("value", value);
    emit Changed(notification);

    return createReply(QVariantMap());
}

JsonReply* AppDataHandler::Load(const QVariantMap &params)
{
    QString appId = params.value("appId").toString();
    QString group = params.value("group").toString();
    QString key = params.value("key").toString();

    QSettings settings(NymeaSettings::storagePath() + "/appdata/" + appId + '/' + group + ".conf", QSettings::IniFormat);

    QVariantMap returns;

    returns.insert("value", settings.value(key).toString());
    return createReply(returns);
}
