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

#ifndef PUSHBUTTONDBUSSERVICE_H
#define PUSHBUTTONDBUSSERVICE_H

#ifdef WITH_DBUS

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

#endif // WITH_DBUS

#endif // PUSHBUTTONDBUSSERVICE_H
