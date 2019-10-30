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

#ifndef EXPERIENCEMANAGER_H
#define EXPERIENCEMANAGER_H

#include <QObject>

class ExperiencePlugin;
class JsonRPCServer;
class DeviceManager;

namespace nymeaserver {


class ExperienceManager : public QObject
{
    Q_OBJECT
public:
    explicit ExperienceManager(DeviceManager *deviceManager, JsonRPCServer *jsonRpcServer, QObject *parent = nullptr);

signals:

public slots:

private slots:
    void loadPlugins();

private:
    QStringList pluginSearchDirs() const;

private:
    DeviceManager *m_deviceManager = nullptr;
    JsonRPCServer *m_jsonRpcServer = nullptr;

    void loadExperiencePlugin(const QString &file);

private:
    QList<ExperiencePlugin*> m_plugins;
};

}
#endif // EXPERIENCEMANAGER_H
