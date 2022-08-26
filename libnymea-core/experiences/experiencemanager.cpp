/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "experiencemanager.h"
#include "experiences/experienceplugin.h"

#include "jsonrpc/jsonrpcserverimplementation.h"
#include "loggingcategories.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>

namespace nymeaserver {

ExperienceManager::ExperienceManager(ThingManager *thingManager, JsonRPCServer *jsonRpcServer, QObject *parent) : QObject(parent),
    m_thingManager(thingManager),
    m_jsonRpcServer(jsonRpcServer)
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
    plugin->initPlugin(m_thingManager, m_jsonRpcServer);

}

void ExperienceManager::loadExperiencePlugin(ExperiencePlugin *experiencePlugin)
{
    qCDebug(dcExperiences()) << "Adding experience plugin:" << experiencePlugin;
    m_plugins.append(experiencePlugin);
    experiencePlugin->setParent(this);
    experiencePlugin->initPlugin(m_thingManager, m_jsonRpcServer);
}

}
