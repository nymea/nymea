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

#ifndef NYMEABLUETOOTHAGENT_H
#define NYMEABLUETOOTHAGENT_H

#include <QBluetoothAddress>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QObject>

#include <hardware/bluetoothlowenergy/nymeabluetoothagent.h>

namespace nymeaserver {

class NymeaBluetoothAgent;

class NymeaBluetoothAgentAdapter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")

public:
    explicit NymeaBluetoothAgentAdapter(NymeaBluetoothAgent *agent, QObject *parent = nullptr);

public slots:
    Q_SCRIPTABLE QString RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &message);
    Q_SCRIPTABLE void DisplayPinCode(const QDBusObjectPath &device, const QString &pinCode);
    Q_SCRIPTABLE quint32 RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &message);
    Q_SCRIPTABLE void DisplayPasskey(const QDBusObjectPath &device, quint32 passKey, quint16 entered);

    Q_SCRIPTABLE void RequestConfirmation(const QDBusObjectPath &device, quint32 passKey, const QDBusMessage &message);
    Q_SCRIPTABLE void RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &message);
    Q_SCRIPTABLE void AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &message);

    Q_SCRIPTABLE void Cancel();
    Q_SCRIPTABLE void Release();

private:
    NymeaBluetoothAgent *m_agent = nullptr;
};

class NymeaBluetoothAgent : public QObject
{
    Q_OBJECT

public:
    explicit NymeaBluetoothAgent(QObject *parent = nullptr);

    void passKeyEntered(const QBluetoothAddress &address, const QString passKey);

signals:
    void passKeyRequested(const QBluetoothAddress &address);
    void displayPinCode(const QBluetoothAddress &address, const QString &pinCode);

private:
    friend class NymeaBluetoothAgentAdapter;
    QBluetoothAddress deviceForPath(const QDBusObjectPath &path);
    void onRequestPassKey(const QDBusObjectPath &path, const QDBusMessage &message);
    void onDisplayPinCode(const QDBusObjectPath &path, const QString &pinCode);
    NymeaBluetoothAgentAdapter *m_adapter = nullptr;
    QHash<QString, QDBusMessage> m_pendingPairings;
};

} // namespace nymeaserver

#endif // NYMEABLUETOOTHAGENT_H
