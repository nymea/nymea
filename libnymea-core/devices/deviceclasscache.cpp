#include "deviceclasscache.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#include "loggingcategories.h"

PluginInfoCache::PluginInfoCache()
{

}

void PluginInfoCache::cachePluginInfo(const QJsonObject &metaData)
{
    QString fileName = metaData.value("id").toString().remove(QRegExp("[{}]")) + ".cache";
    QDir path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/plugininfo/";
    if (!path.exists()) {
        if (!path.mkpath(path.absolutePath())) {
            qCWarning(dcDeviceManager()) << "Error creating device class cache dir at" << path.absolutePath();
        }
    }
    QFile file(path.absoluteFilePath(fileName));
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(dcDeviceManager()) << "Error opening device class cache for writing at" << path.absoluteFilePath(fileName);
        return;
    }

    file.write(QJsonDocument::fromVariant(metaData.toVariantMap()).toJson(QJsonDocument::Compact));
    file.close();
}

QJsonObject PluginInfoCache::loadPluginInfo(const PluginId &pluginId)
{
    QString fileName = pluginId.toString().remove(QRegExp("[{}]")) + ".cache";
    QDir path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/plugininfo/";
    QFile file(path.absoluteFilePath(fileName));
    if (!file.open(QFile::ReadOnly)) {
        return QJsonObject();
    }

    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcDeviceManager()) << "Error parsing plugin info cache entry:" << path.absoluteFilePath(fileName);
        return QJsonObject();
    }
    return QJsonObject::fromVariantMap(jsonDoc.toVariant().toMap());
}
