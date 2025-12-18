// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "experiencemanager.h"
#include "experiences/experienceplugin.h"

#include "jsonrpc/jsonrpcserverimplementation.h"
#include "loggingcategories.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>

namespace nymeaserver {

ExperienceManager::ExperienceManager(ThingManager *thingManager, JsonRPCServer *jsonRpcServer, QObject *parent)
    : QObject(parent)
    , m_thingManager(thingManager)
    , m_jsonRpcServer(jsonRpcServer)
{
    staticMetaObject.invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

QList<ExperiencePlugin *> ExperienceManager::plugins() const
{
    return m_plugins;
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
                    loadExperiencePlugin(path + "/" + entry + "/libnymea_experienceplugin" + entry + ".so");
                }
            }
        }
    }
}

QStringList ExperienceManager::pluginSearchDirs() const
{
    const char *envDefaultPath = "NYMEA_EXPERIENCE_PLUGINS_PATH";
    const char *envExtraPath = "NYMEA_EXPERIENCE_PLUGINS_EXTRA_PATH";

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
            searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "experiences");
        }
#else
        foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
            searchDirs << libraryPath.replace("qt6", "nymea").replace("plugins", "experiences");
        }
#endif
        searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../lib/nymea/experiences").absolutePath();
        searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../experiences/").absolutePath();
        searchDirs << QDir(QCoreApplication::applicationDirPath() + "/../../../experiences/").absolutePath();
    }

    searchDirs.removeDuplicates();
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
    ExperiencePlugin *plugin = qobject_cast<ExperiencePlugin *>(loader.instance());
    if (!plugin) {
        qCWarning(dcExperiences()) << "Could not get plugin instance of" << loader.fileName();
        loader.unload();
        return;
    }
    qCDebug(dcExperiences()) << "Loaded experience plugin:" << loader.fileName();
    m_plugins.append(plugin);
    plugin->setParent(this);
    plugin->initPlugin(m_thingManager, m_jsonRpcServer);
}

void ExperienceManager::loadExperiencePlugin(ExperiencePlugin *experiencePlugin)
{
    qCDebug(dcExperiences()) << "Adding experience plugin:" << experiencePlugin;
    m_plugins.append(experiencePlugin);
    experiencePlugin->setParent(this);
    experiencePlugin->initPlugin(m_thingManager, m_jsonRpcServer);
}

} // namespace nymeaserver
