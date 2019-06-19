/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "platform.h"
#include "platform/platformsystemcontroller.h"
#include "platform/platformupdatecontroller.h"

#include "loggingcategories.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

namespace nymeaserver {

Platform::Platform(QObject *parent) : QObject(parent)
{
    foreach (const QString &path, pluginSearchDirs()) {
        QDir dir(path);
        qCDebug(dcPlatform) << "Loading platform plugins from:" << dir.absolutePath();
        foreach (const QString &entry, dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFileInfo fi(path + "/" + entry);
            if (fi.isFile()) {
                if (entry.startsWith("libnymea_systemplugin") && entry.endsWith(".so")) {
                    loadSystemPlugin(path + "/" + entry);
                } else if (entry.startsWith("libnymea_updateplugin") && entry.endsWith(".so")) {
                    loadUpdatePlugin(path + "/" + entry);
                }
            } else if (fi.isDir()) {
                if (QFileInfo::exists(path + "/" + entry + "/libnymea_systemplugin" + entry + ".so")) {
                    loadSystemPlugin(path + "/" +  entry + "/libnymea_systemplugin" + entry + ".so");
                } else if (QFileInfo::exists(path + "/" + entry + "/libnymea_updateplugin" + entry + ".so")) {
                    loadUpdatePlugin(path + "/" +  entry + "/libnymea_updateplugin" + entry + ".so");
                }
            }
        }
        if (m_platformSystemController && m_platformUpdateController) {
            break;
        }
    }
    if (!m_platformSystemController) {
        qCWarning(dcPlatform()) << "Could not load a system plugin. System control features won't be available.";
        m_platformSystemController = new PlatformSystemController(this);
    }
    if (!m_platformUpdateController) {
        qCWarning(dcPlatform()) << "Could not load an update plugin. System update features won't be available.";
        m_platformUpdateController = new PlatformUpdateController(this);
    }
}

PlatformSystemController *Platform::systemController() const
{
    return m_platformSystemController;
}

PlatformUpdateController *Platform::updateController() const
{
    return m_platformUpdateController;
}

QStringList Platform::pluginSearchDirs() const
{
    QStringList searchDirs;
    QByteArray envPath = qgetenv("NYMEA_PLATFORM_PLUGINS_PATH");
    if (!envPath.isEmpty()) {
        searchDirs << QString(envPath).split(':');
    }

    foreach (QString libraryPath, QCoreApplication::libraryPaths()) {
        searchDirs << libraryPath.replace("qt5", "nymea").replace("plugins", "platform");
    }
    searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/platform";
    searchDirs << QCoreApplication::applicationDirPath() + "/../platform/";
    searchDirs << QCoreApplication::applicationDirPath() + "/../../../platform/";
    return searchDirs;
}

void Platform::loadSystemPlugin(const QString &file)
{
    if (m_platformSystemController) {
        return; // Not loading another...
    }
    QPluginLoader loader;
    loader.setFileName(file);
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (!loader.load()) {
        qCWarning(dcPlatform) << "Could not load plugin data of" << file << "\n" << loader.errorString();
        return;
    }
    m_platformSystemController = qobject_cast<PlatformSystemController*>(loader.instance());
    if (!m_platformSystemController) {
        qCWarning(dcPlatform) << "Could not get plugin instance of" << file;
        loader.unload();
        return;
    }
    qCDebug(dcPlatform()) << "Loaded system plugin:" << file;
    m_platformSystemController->setParent(this);
}

void Platform::loadUpdatePlugin(const QString &file)
{
    if (m_platformUpdateController) {
        return; // Not loading another...
    }
    QPluginLoader loader;
    loader.setFileName(file);
    loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (!loader.load()) {
        qCWarning(dcPlatform) << "Could not load plugin data of" << file << "\n" << loader.errorString();
        return;
    }
    m_platformUpdateController = qobject_cast<PlatformUpdateController*>(loader.instance());
    if (!m_platformUpdateController) {
        qCWarning(dcPlatform) << "Could not get plugin instance of" << file;
        loader.unload();
        return;
    }
    qCDebug(dcPlatform()) << "Loaded update plugin:" << file;
    m_platformUpdateController->setParent(this);
}

}
