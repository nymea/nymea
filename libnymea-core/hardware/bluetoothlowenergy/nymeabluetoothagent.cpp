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

#include "nymeabluetoothagent.h"

#include <QString>
#include <QDBusConnection>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcBluetooth)

namespace nymeaserver {

NymeaBluetoothAgentAdapter::NymeaBluetoothAgentAdapter(NymeaBluetoothAgent *agent, QObject *parent):
    QObject{parent},
    m_agent{agent}
{

}

QString NymeaBluetoothAgentAdapter::RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &message)
{
    qCDebug(dcBluetooth) << "RequestPinCode" << device.path() << message.arguments();
    message.setDelayedReply(true);
    m_agent->onRequestPassKey(device, message);
    return 0;
}

void NymeaBluetoothAgentAdapter::DisplayPinCode(const QDBusObjectPath &device, const QString &pinCode)
{
    qCDebug(dcBluetooth) << "DisplayPinCode" << device.path() << pinCode;
    m_agent->onDisplayPinCode(device, pinCode);
}

quint32 NymeaBluetoothAgentAdapter::RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &message)
{
    qCDebug(dcBluetooth) << "RequestPasskey" << device.path() << message.arguments();
    message.setDelayedReply(true);
    m_agent->onRequestPassKey(device, message);
    return 0;
}

void NymeaBluetoothAgentAdapter::DisplayPasskey(const QDBusObjectPath &device, quint32 passKey, quint16 entered)
{
    qCDebug(dcBluetooth) << "DisplayPasskey" << device.path() << passKey << entered;
    // Not using "entered" value which would update whenever the user enters a (wrong) pin on the other end...
    m_agent->onDisplayPinCode(device, QString("%1").arg(passKey, 6, QChar('0')));
}

void NymeaBluetoothAgentAdapter::RequestConfirmation(const QDBusObjectPath &device, quint32 passKey, const QDBusMessage &message)
{
    qCDebug(dcBluetooth) << "RequestConfirmation" << device.path() << passKey << message.arguments();
    // TODO: Not implemented
    qCWarning(dcBluetooth()) << "Request confirmation pairing mechanism is not implemented.";
}

void NymeaBluetoothAgentAdapter::RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &message)
{
    qCDebug(dcBluetooth) << "RequestAuthorization" << device.path() << message.arguments();
    // TODO: Not implemented
    qCWarning(dcBluetooth()) << "RequestAuthorization mechanism is not implemented.";
}

void NymeaBluetoothAgentAdapter::AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &message)
{
    qCDebug(dcBluetooth) << "AuthorizeService" << device.path() << uuid << message.arguments();
    // TODO: Not implemented
    qCWarning(dcBluetooth()) << "AuthorizeService mechanism is not implemented.";

}

void NymeaBluetoothAgentAdapter::Cancel()
{
    qCDebug(dcBluetooth()) << "Cancel called on bluetooth agent";
}

void NymeaBluetoothAgentAdapter::Release()
{
    qCDebug(dcBluetooth()) << "Release called on bluetooth agent";
}

NymeaBluetoothAgent::NymeaBluetoothAgent(QObject *parent)
    : QObject{parent}
{
    m_adapter = new NymeaBluetoothAgentAdapter(this);
    bool success = QDBusConnection::systemBus().registerObject("/nymea/bluetoothagent", m_adapter, QDBusConnection::ExportScriptableContents);
    qCInfo(dcBluetooth) << "Registered Bluetooth pairing agent" << success;

    QDBusMessage message = QDBusMessage::createMethodCall("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RegisterAgent");
    message << QVariant::fromValue(QDBusObjectPath("/nymea/bluetoothagent"));
    message << "KeyboardDisplay";
    QDBusMessage registerReply = QDBusConnection::systemBus().call(message);
    if (!registerReply.errorName().isEmpty()) {
        qCWarning(dcBluetooth()) << "Error registering pairing agent:" << registerReply.errorMessage();
    } else {
        qCDebug(dcBluetooth()) << "Pairing agent registered.";
    }

}

void NymeaBluetoothAgent::passKeyEntered(const QBluetoothAddress &address, const QString passKey)
{
    if (!m_pendingPairings.contains(address.toString())) {
        qCWarning(dcBluetooth()) << "No ongoing pairing process for" << address.toString();
        return;
    }

    qCInfo(dcBluetooth()) << "Providing passkey to" << address.toString() << passKey;
    QDBusMessage message = m_pendingPairings.take(address.toString());
    message << static_cast<quint32>(passKey.toUInt());
    QDBusConnection::systemBus().send(message);
}

QBluetoothAddress NymeaBluetoothAgent::deviceForPath(const QDBusObjectPath &path)
{
//   qdbus --system org.bluez /org/bluez/hci0/dev_00_1A_22_0B_12_EB org.freedesktop.DBus.Properties.Get org.bluez.Device1 Address

    QDBusMessage message = QDBusMessage::createMethodCall("org.bluez", path.path(), "org.freedesktop.DBus.Properties", "Get");
    message << "org.bluez.Device1" << "Address";
    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    if (!reply.errorName().isEmpty()) {
        qCWarning(dcBluetooth()) << "Error reading Address property for" << path.path() << reply.errorMessage();
        return QBluetoothAddress();
    }

    if (reply.arguments().count() != 1) {
        qCWarning(dcBluetooth) << "Read property reply received an unexpected argument count";
        return QBluetoothAddress();
    }

    return QBluetoothAddress(reply.arguments().at(0).value<QDBusVariant>().variant().toString());
}

void NymeaBluetoothAgent::onRequestPassKey(const QDBusObjectPath &path, const QDBusMessage &message)
{
    QBluetoothAddress address = deviceForPath(path);
    qCDebug(dcBluetooth()) << "RequestPassKey" << path.path() << address;
    m_pendingPairings[address.toString()] = message.createReply();
    emit passKeyRequested(address);
}

void NymeaBluetoothAgent::onDisplayPinCode(const QDBusObjectPath &path, const QString &pinCode)
{
    QBluetoothAddress address = deviceForPath(path);
    qCDebug(dcBluetooth()) << "RequestPassKey" << path.path() << address;
    emit displayPinCode(address, pinCode);
}

}
