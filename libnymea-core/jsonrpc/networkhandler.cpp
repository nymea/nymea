// SPDX-License-Identifier: LGPL-3.0-or-later
#include "networkhandler.h"

#include "nymeasettings.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QAbstractSocket>

#include <algorithm>

namespace nymeaserver {

static QString tunKind(const QString &tunFlagsPath)
{
    QFile file(tunFlagsPath);
    if (!file.open(QFile::ReadOnly)) {
        return "tun";
    }

    bool ok = false;
    const uint flags = file.readAll().trimmed().toUInt(&ok, 0);
    // IFF_TAP from linux/if_tun.h
    return ok && (flags & 0x0002) ? "tap" : "tun";
}

NetworkHandler::NetworkHandler(QObject *parent) : JsonHandler(parent)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
    settings.beginGroup("Network");
    m_backendChoice = settings.value("Backend", "auto").toString().toLower();
    settings.endGroup();

    QVariantMap address;
    address.insert("address", enumValueName(String));
    address.insert("prefixLength", enumValueName(Int));
    registerObject("NetworkAddress", address);

    QVariantMap networkInterface;
    networkInterface.insert("index", enumValueName(Int));
    networkInterface.insert("name", enumValueName(String));
    networkInterface.insert("kind", enumValueName(String));
    networkInterface.insert("macAddress", enumValueName(String));
    networkInterface.insert("administrativeState", enumValueName(String));
    networkInterface.insert("operationalState", enumValueName(String));
    networkInterface.insert("managementStatus", enumValueName(String));
    networkInterface.insert("ipv4Addresses", QVariantList() << objectRef("NetworkAddress"));
    networkInterface.insert("ipv6Addresses", QVariantList() << objectRef("NetworkAddress"));
    registerObject("NetworkInterface", networkInterface);

    QVariantMap info;
    info.insert("backend", enumValueName(String));
    info.insert("available", enumValueName(Bool));
    info.insert("features", enumValueName(StringList));
    registerObject("NetworkBackendInfo", info);

    QVariantMap params;
    QVariantMap returns;
    returns.insert("backendInfo", objectRef("NetworkBackendInfo"));
    registerMethod("GetBackendInfo", "Get the selected networking backend and its availability.", params, returns);
    params.insert("backendInfo", objectRef("NetworkBackendInfo"));
    registerNotification("BackendInfoChanged", "The selected networking backend or its availability changed.", params);

    params.clear(); returns.clear();
    returns.insert("interfaces", QVariantList() << objectRef("NetworkInterface"));
    registerMethod("GetInterfaces", "Get all visible network interfaces and their addresses.", params, returns);
    params.insert("interfaces", QVariantList() << objectRef("NetworkInterface"));
    registerNotification("InterfacesChanged", "The network interface snapshot changed.", params);

    connect(&m_refreshTimer, &QTimer::timeout, this, &NetworkHandler::refresh);
    m_refreshTimer.setInterval(2000);
    refresh();
    m_refreshTimer.start();
}

QString NetworkHandler::name() const
{
    return "Network";
}

JsonReply *NetworkHandler::GetBackendInfo(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap result;
    result.insert("backendInfo", m_backendInfo);
    return createReply(result);
}

JsonReply *NetworkHandler::GetInterfaces(const QVariantMap &params)
{
    Q_UNUSED(params)
    QVariantMap result;
    result.insert("interfaces", m_interfaces);
    return createReply(result);
}

QVariantMap NetworkHandler::backendInfo() const
{
    QDBusConnectionInterface *bus = QDBusConnection::systemBus().interface();
    const bool managerAvailable = bus && bus->isServiceRegistered("org.freedesktop.NetworkManager");
    const bool networkdAvailable = bus && bus->isServiceRegistered("org.freedesktop.network1");

    const QString backend = m_selectedBackend.isEmpty() ? "none" : m_selectedBackend;
    const bool available = (backend == "networkmanager" && managerAvailable)
            || (backend == "networkd" && networkdAvailable);

    QVariantMap info;
    info.insert("backend", backend);
    info.insert("available", available);
    info.insert("features", QStringList() << "interfaceReporting");
    return info;
}

QVariantList NetworkHandler::interfaces() const
{
    QList<QNetworkInterface> links = QNetworkInterface::allInterfaces();
    std::sort(links.begin(), links.end(), [](const QNetworkInterface &left, const QNetworkInterface &right) {
        return left.index() < right.index();
    });

    QVariantList result;
    foreach (const QNetworkInterface &link, links) {
        QVariantMap entry;
        entry.insert("index", link.index());
        entry.insert("name", link.name());
        QString kind = "unknown";
        const QString sysfsPath = QStringLiteral("/sys/class/net/") + link.name();
        if (link.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            kind = "loopback";
        } else if (QFileInfo::exists(sysfsPath + "/wireless")) {
            kind = "wifi";
        } else if (QFileInfo::exists(sysfsPath + "/bridge")) {
            kind = "bridge";
        } else if (QFileInfo::exists(QStringLiteral("/proc/net/vlan/") + link.name())) {
            kind = "vlan";
        } else if (QFileInfo::exists(sysfsPath + "/tun_flags")) {
            kind = tunKind(sysfsPath + "/tun_flags");
        } else if (QFileInfo::exists(sysfsPath + "/device")) {
            kind = "ethernet";
        }
        entry.insert("kind", kind);
        entry.insert("macAddress", link.hardwareAddress());
        entry.insert("administrativeState", link.flags().testFlag(QNetworkInterface::IsUp) ? "up" : "down");
        entry.insert("operationalState", link.flags().testFlag(QNetworkInterface::IsRunning) ? "up" : "down");
        entry.insert("managementStatus", "unknown");

        QVariantList ipv4;
        QVariantList ipv6;
        foreach (const QNetworkAddressEntry &address, link.addressEntries()) {
            // The scope id (e.g. %eth0) is implied by the interface
            QHostAddress ip = address.ip();
            ip.setScopeId(QString());
            QVariantMap item;
            item.insert("address", ip.toString());
            item.insert("prefixLength", address.prefixLength());
            if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                ipv4.append(item);
            } else if (address.ip().protocol() == QAbstractSocket::IPv6Protocol) {
                ipv6.append(item);
            }
        }
        entry.insert("ipv4Addresses", ipv4);
        entry.insert("ipv6Addresses", ipv6);
        result.append(entry);
    }
    return result;
}

void NetworkHandler::refresh()
{
    QDBusConnectionInterface *bus = QDBusConnection::systemBus().interface();
    const bool managerAvailable = bus && bus->isServiceRegistered("org.freedesktop.NetworkManager");
    const bool networkdAvailable = bus && bus->isServiceRegistered("org.freedesktop.network1");
    if (m_backendChoice == "networkmanager" || m_backendChoice == "networkd") {
        m_selectedBackend = m_backendChoice;
    } else if (m_backendChoice == "auto") {
        if (managerAvailable) {
            m_selectedBackend = "networkmanager";
        } else if (m_selectedBackend.isEmpty() && networkdAvailable) {
            m_selectedBackend = "networkd";
        }
    }

    const QVariantMap currentBackendInfo = backendInfo();
    if (currentBackendInfo != m_backendInfo) {
        m_backendInfo = currentBackendInfo;
        QVariantMap params;
        params.insert("backendInfo", m_backendInfo);
        emit BackendInfoChanged(params);
    }

    const QVariantList currentInterfaces = interfaces();
    if (currentInterfaces != m_interfaces) {
        m_interfaces = currentInterfaces;
        QVariantMap params;
        params.insert("interfaces", m_interfaces);
        emit InterfacesChanged(params);
    }
}

}
