#include "apikeysprovidersloader.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

ApiKeysProvidersLoader::ApiKeysProvidersLoader(QObject *parent):
    QObject(parent)
{

    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcApiKeys()) << "Loading API keys provider plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFileInfo fi(path + "/" + entry);
            if (fi.isFile()) {
                if (entry.startsWith("libnymea_apikeysproviderplugin") && entry.endsWith(".so")) {
                    loadPlugin(path + "/" + entry);
                }
            } else if (fi.isDir()) {
                if (QFileInfo::exists(path + "/" + entry + "/libnymea_apikeysproviderplugin" + entry + ".so")) {
                    loadPlugin(path + "/" +  entry + "/libnymea_apikeysproviderplugin" + entry + ".so");
                }
            }
        }
    }
}

QHash<QString, ApiKey> ApiKeysProvidersLoader::allApiKeys() const
{
    QHash<QString, ApiKey> ret;
    foreach (ApiKeysProvider *provider, m_providers) {
        foreach (const QString &name, provider->apiKeys().keys()) {
            ret.insert(name, provider->apiKeys().value(name));
        }
    }
    return ret;
}

QStringList ApiKeysProvidersLoader::pluginSearchDirs() const
{
    const char *envDefaultPath = "NYMEA_APIKEYS_PLUGINS_PATH";
    const char *envExtraPath = "NYMEA_APIKEYS_PLUGINS_EXTRA_PATH";

    QStringList searchDirs;
    QByteArray envExtraPathData = qgetenv(envExtraPath);
    if (!envExtraPathData.isEmpty()) {
        searchDirs << QString::fromUtf8(envExtraPathData).split(':');
    }

    if (qEnvironmentVariableIsSet(envDefaultPath)) {
        QByteArray envDefaultPathData = qgetenv(envDefaultPath);
        if (!envDefaultPathData.isEmpty()) {
            searchDirs << QString::fromUtf8(envDefaultPathData).split(':');
        }
    } else {
        foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
            searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "apikeysproviders");
        }
        searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../lib/nymea/apikeysproviders").absolutePath();
        searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../apikeysproviders/").absolutePath();
        searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../../../apikeysproviders/").absolutePath();
    }

    searchDirs.removeDuplicates();
    return searchDirs;
}

void ApiKeysProvidersLoader::loadPlugin(const QString &file)
{
    QPluginLoader loader;
    loader.setFileName(file);
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (!loader.load()) {
        qCWarning(dcApiKeys()) << loader.errorString();
        return;
    }
    ApiKeysProvider *provider = qobject_cast<ApiKeysProvider*>(loader.instance());
    if (!provider) {
        qCWarning(dcApiKeys()) << "Could not get plugin instance of" << loader.fileName();
        loader.unload();
        return;
    }
    qCDebug(dcApiKeys()) << "Loaded API keys provider plugin:" << loader.fileName();
    provider->setParent(this);
    m_providers.append(provider);

}
