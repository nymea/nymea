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

#include "experienceplugin.h"

ExperiencePlugin::ExperiencePlugin(QObject *parent) : QObject(parent)
{

}

/*! This method will be called when the plugin has been completely loaded and experience
    logic may start operating. A plugin can reimplment this to do initialisation code. */
void ExperiencePlugin::init()
{

}

/*! Returns a pointer to the DeviceManager. The pointer won't be valid unless init() has been called. */
DeviceManager *ExperiencePlugin::deviceManager()
{
    return m_deviceManager;
}

/*! Returns a pointer to the JsonRPCServer. The pointer won't be valid unless init() has been called. */
JsonRPCServer *ExperiencePlugin::jsonRpcServer()
{
    return m_jsonRpcServer;
}


void ExperiencePlugin::initPlugin(DeviceManager *deviceManager, JsonRPCServer *jsonRPCServer)
{
    m_deviceManager = deviceManager;
    m_jsonRpcServer = jsonRPCServer;

    init();
}
