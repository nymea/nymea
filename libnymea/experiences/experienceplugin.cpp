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

#include "experienceplugin.h"

ExperiencePlugin::ExperiencePlugin(QObject *parent) : QObject(parent)
{

}

/*! This method will be called when the plugin has been completely loaded and experience
    logic may start operating. A plugin can reimplment this to do initialisation code. */
void ExperiencePlugin::init()
{

}

/*! This method will can be used to provide a web server resource to the core.
  Override this method and provide an object. The resource will be added to the webserver after the init() method has been called. */
WebServerResource *ExperiencePlugin::webServerResource() const
{
    return nullptr;
}

/*! Returns a pointer to the DeviceManager. The pointer won't be valid unless init() has been called. */
ThingManager *ExperiencePlugin::thingManager()
{
    return m_thingManager;
}

/*! Returns a pointer to the JsonRPCServer. The pointer won't be valid unless init() has been called. */
JsonRPCServer *ExperiencePlugin::jsonRpcServer()
{
    return m_jsonRpcServer;
}

LogEngine *ExperiencePlugin::logEngine()
{
    return m_logEngine;
}

void ExperiencePlugin::initPlugin(ThingManager *deviceManager, JsonRPCServer *jsonRPCServer, LogEngine *logEngine)
{
    m_thingManager = deviceManager;
    m_jsonRpcServer = jsonRPCServer;
    m_logEngine = logEngine;

    init();
}
