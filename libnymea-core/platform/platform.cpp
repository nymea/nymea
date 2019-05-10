#include "platform.h"
#include "platform/platformplugin.h"

#include "loggingcategories.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

namespace nymeaserver {

Platform::Platform(QObject *parent) : QObject(parent)
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcPlatform) << "Loading plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            qCDebug(dcPlatform()) << "Found dir entry" << entry;
            QFileInfo fi;
            if (entry.startsWith("libnymea_platformplugin") && entry.endsWith(".so")) {
                fi.setFile(path + "/" + entry);
            } else {
                fi.setFile(path + "/" + entry + "/libnymea_platformplugin" + entry + ".so");
            }

            if (!fi.exists())
                continue;


            QPluginLoader loader;
            loader.setFileName(fi.absoluteFilePath());
            loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

            if (!loader.load()) {
                qCWarning(dcPlatform) << "Could not load plugin data of" << entry << "\n" << loader.errorString();
                continue;
            }

            m_platformPlugin = qobject_cast<PlatformPlugin *>(loader.instance());
            if (!m_platformPlugin) {
                qCWarning(dcPlatform) << "Could not get plugin instance of" << entry;
                continue;
            }
            qCDebug(dcPlatform()) << "Loaded platform plugin:" << entry;
            m_platformPlugin->setParent(this);
            break;
        }
        if (m_platformPlugin) {
            break;
        }
    }
    if (!m_platformPlugin) {
        qCWarning(dcPlatform()) << "Could not load a platform plugin. Platform related features won't be available.";
        m_platformPlugin = new PlatformPlugin(this);
    }

}

PlatformSystemController *Platform::systemController() const
{
    return m_platformPlugin->systemController();
}

PlatformUpdateController *Platform::updateController() const
{
    return m_platformPlugin->updateController();
}

QStringList Platform::pluginSearchDirs() const
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_PLATFORM_PLUGINS_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "platforms");
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/platforms";
    searchDirs << QCoreApplication::applicationDirPath() + "/../platforms/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../platforms/";
    return searchDirs;
}

}
