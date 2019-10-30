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

#ifndef EXPERIENCEPLUGIN_H
#define EXPERIENCEPLUGIN_H

#include <QObject>

class DeviceManager;
class JsonRPCServer;

namespace nymeaserver {
class ExperienceManager;
}
class ExperiencePlugin : public QObject
{
    Q_OBJECT
public:
    explicit ExperiencePlugin(QObject *parent = nullptr);

    virtual void init() = 0;

protected:
    DeviceManager* deviceManager();
    JsonRPCServer* jsonRpcServer();

private:
    friend class nymeaserver::ExperienceManager;
    void initPlugin(DeviceManager *deviceManager, JsonRPCServer *jsonRPCServer);

    DeviceManager *m_deviceManager = nullptr;
    JsonRPCServer *m_jsonRpcServer = nullptr;

};

Q_DECLARE_INTERFACE(ExperiencePlugin, "io.nymea.ExperiencePlugin")


#endif // EXPERIENCEPLUGIN_H
