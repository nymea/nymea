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

#include "pushbuttondbusservice.h"
#include "loggingcategories.h"
#include "nymeadbusservice.h"

#include <QDBusConnection>
#include <QDebug>
#include <QDBusServiceWatcher>
#include <QDBusMessage>

namespace nymeaserver {

PushButtonDBusService::PushButtonDBusService(const QString &objectPath, UserManager *parent) :
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
    QMetaObject::Connection c = connect(m_userManager, &UserManager::pushButtonAuthFinished, this, [&] (int i, bool s, const QByteArray &t) {
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
