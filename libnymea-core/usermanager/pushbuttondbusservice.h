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

#ifndef PUSHBUTTONDBUSSERVICE_H
#define PUSHBUTTONDBUSSERVICE_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusContext>

#include "usermanager.h"

namespace nymeaserver {

class PushButtonDBusService : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit PushButtonDBusService(const QString &objectPath, UserManager *parent);

    bool agentAvailable() const;

public slots:
    Q_SCRIPTABLE void RegisterButtonAgent(const QDBusObjectPath &agentPath);
    Q_SCRIPTABLE void UnregisterButtonAgent();

    Q_SCRIPTABLE QByteArray GenerateAuthToken(const QString &deviceName);

signals:
    void pushButtonPressed();

private slots:
    void serviceUnregistered(const QString &serviceName);

private:
    UserManager *m_userManager;
    QStringList m_registeredAgents;
};

}

#endif // PUSHBUTTONDBUSSERVICE_H
