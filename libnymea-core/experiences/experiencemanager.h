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

#ifndef EXPERIENCEMANAGER_H
#define EXPERIENCEMANAGER_H

#include <QObject>

class ExperiencePlugin;
class JsonRPCServer;
class ThingManager;

namespace nymeaserver {

class ExperienceManager : public QObject
{
    Q_OBJECT
public:
    explicit ExperienceManager(ThingManager *thingManager, JsonRPCServer *jsonRpcServer, QObject *parent = nullptr);

    QList<ExperiencePlugin *> plugins() const;

    // This method is used for testing
    void loadExperiencePlugin(ExperiencePlugin *experiencePlugin);

private slots:
    void loadPlugins();

private:
    ThingManager *m_thingManager = nullptr;
    JsonRPCServer *m_jsonRpcServer = nullptr;
    QList<ExperiencePlugin *> m_plugins;

    QStringList pluginSearchDirs() const;
    void loadExperiencePlugin(const QString &file);
};

} // namespace nymeaserver
#endif // EXPERIENCEMANAGER_H
