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

#include "pushbuttondbusservice.h"
#include "loggingcategories.h"
#include "nymeadbusservice.h"

#include <QDBusConnection>
#include <QDebug>
#include <QDBusServiceWatcher>
#include <QDBusMessage>

namespace nymeaserver {

PushButtonDBusService::PushButtonDBusService(const QString &objectPath, BuiltinUserBackend *parent) :
    QObject(parent),
    m_userManager(parent)
{
    NymeaDBusService* dbusService = new NymeaDBusService(objectPath, this);
    if (!dbusService->isValid()) {
        qCWarning(dcUserManager()) << "Failed to register PushButton D-Bus service.";
        return;
    }
    qCDebug(dcUserManager()) << "PushButton D-Bus service set up.";
}

bool PushButtonDBusService::agentAvailable() const
{
    return m_registeredAgents.count() > 0;
}

void PushButtonDBusService::RegisterButtonAgent(const QDBusObjectPath &agentPath)
{
    QDBusMessage msg = message();
    connection().connect(QString(), QString(agentPath.path()), QString(), "PushButtonPressed", this, SIGNAL(pushButtonPressed()));
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(msg.service(), connection(), QDBusServiceWatcher::WatchForUnregistration, this);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &PushButtonDBusService::serviceUnregistered);
    qCDebug(dcUserManager()) << "PushButton handler" << agentPath.path() << msg.service() << "registered.";
    m_registeredAgents.append(msg.service());
}

void PushButtonDBusService::UnregisterButtonAgent()
{
    QDBusMessage msg = message();
    serviceUnregistered(msg.service());
}

QByteArray PushButtonDBusService::GenerateAuthToken(const QString &deviceName)
{
    int transactionId = m_userManager->requestPushButtonAuth(deviceName);
    bool success = false;
    QByteArray token;
    QMetaObject::Connection c = connect(m_userManager, &BuiltinUserBackend::pushButtonAuthFinished, this, [&] (int i, bool s, const QByteArray &t) {
        if (transactionId == i) {
            success = s;
            token = t;
        }
    });
    emit pushButtonPressed();
    disconnect(c);
    return token;
}

void PushButtonDBusService::serviceUnregistered(const QString &serviceName)
{
    if (!m_registeredAgents.contains(serviceName)) {
        qCWarning(dcUserManager()) << "PushButton agent" << serviceName << "not known. Cannot unregister.";
        return;
    }
    m_registeredAgents.removeAll(serviceName);
    qCDebug(dcUserManager()) << "PushButton agent" << serviceName << "unregistered";
}

}
