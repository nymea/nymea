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
