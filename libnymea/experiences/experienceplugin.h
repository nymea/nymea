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

#ifndef EXPERIENCEPLUGIN_H
#define EXPERIENCEPLUGIN_H

#include <QObject>

class LogEngine;
class ThingManager;
class JsonRPCServer;
class WebServerResource;

namespace nymeaserver {

class ExperienceManager;
}
class ExperiencePlugin : public QObject
{
    Q_OBJECT
public:
    explicit ExperiencePlugin(QObject *parent = nullptr);
    virtual ~ExperiencePlugin() = default;

    virtual void init() = 0;

    virtual WebServerResource *webServerResource() const;

protected:
    ThingManager *thingManager();
    JsonRPCServer *jsonRpcServer();
    LogEngine *logEngine();

private:
    friend class nymeaserver::ExperienceManager;
    void initPlugin(ThingManager *thingManager, JsonRPCServer *jsonRPCServer, LogEngine *logEngine = nullptr);

    ThingManager *m_thingManager = nullptr;
    JsonRPCServer *m_jsonRpcServer = nullptr;
    LogEngine *m_logEngine = nullptr;

};

Q_DECLARE_INTERFACE(ExperiencePlugin, "io.nymea.ExperiencePlugin")

#endif // EXPERIENCEPLUGIN_H
