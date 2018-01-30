/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PUSHBUTTONDBUSSERVICE_H
#define PUSHBUTTONDBUSSERVICE_H

#include <QObject>
#include <QDBusObjectPath>

#include "usermanager.h"
#include "nymeadbusservice.h"

namespace nymeaserver {

class PushButtonDBusService : public NymeaDBusService
{
    Q_OBJECT
public:
    explicit PushButtonDBusService(const QString &objectPath, UserManager *parent);

    bool agentAvailable() const;

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
