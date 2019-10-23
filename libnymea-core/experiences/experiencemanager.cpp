#include "experiencemanager.h"
#include "experiences/experienceplugin.h"

#include "jsonrpc/jsonrpcserverimplementation.h"
#include "loggingcategories.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>

namespace nymeaserver {

ExperienceManager::ExperienceManager(DeviceManager *deviceManager, JsonRPCServer *jsonRpcServer, QObject *parent) : QObject(parent),
    m_deviceManager(deviceManager),
    m_jsonRpcServer(jsonRpcServer)
{
    staticMetaObject.invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

void ExperienceManager::loadPlugins()
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcExperiences) << "Loading experience plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFileInfo fi(path + "/" + entry);
            if (fi.isFile()) {
                if (entry.startsWith("libnymea_experienceplugin") && entry.endsWith(".so")) {
                    loadExperiencePlugin(path + "/" + entry);
                }
            } else if (fi.isDir()) {
                if (QFileInfo::exists(path + "/" + entry + "/libnymea_experienceplugin" + entry + ".so")) {
                    loadExperiencePlugin(path + "/" +  entry + "/libnymea_experienceplugin" + entry + ".so");
                }
            }
        }
    }
}

QStringList ExperienceManager::pluginSearchDirs() const
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_EXPERIENCE_PLUGINS_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "experiences");
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/experiences";
    searchDirs << QCoreApplication::applicationDirPath() + "/../experiences/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../experiences/";
    return searchDirs;
}

void ExperienceManager::loadExperiencePlugin(const QString &file)
{
    QPluginLoader loader;
    loader.setFileName(file);
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (!loader.load()) {
        qCWarning(dcExperiences()) << loader.errorString();
        return;
    }
    ExperiencePlugin *plugin = qobject_cast<ExperiencePlugin*>(loader.instance());
    if (!plugin) {
        qCWarning(dcExperiences()) << "Could not get plugin instance of" << loader.fileName();
        loader.unload();
        return;
    }
    qCDebug(dcExperiences()) << "Loaded experience plugin:" << loader.fileName();
    m_plugins.append(plugin);
    plugin->setParent(this);
    plugin->initPlugin(m_deviceManager, m_jsonRpcServer);

}

}
