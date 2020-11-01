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

#include "zigbeemanager.h"
#include "nymeasettings.h"
#include "loggingcategories.h"

#include <zigbeeutils.h>

NYMEA_LOGGING_CATEGORY(dcZigbee, "Zigbee")

// Register debug category from the libnymea-zigbee
NYMEA_LOGGING_CATEGORY(dcZigbeeNetworkLibNymeaZigbee, "ZigbeeNetwork")

namespace nymeaserver {

ZigbeeManager::ZigbeeManager(QObject *parent) :
    QObject(parent)
{
    // Adapter monitor
    qCDebug(dcZigbee()) << "Initialize the ZigBee manager";
    m_adapterMonitor = new ZigbeeUartAdapterMonitor(this);
    if (!m_adapterMonitor->isValid()) {
        qCWarning(dcZigbee()) << "Could not initialize the ZigBee adapter monitor.";
        // Lets continue anyways, maybe we can set up existing networks right the way.
    }

    qCDebug(dcZigbee()) << "Loading initial adapter list";
    foreach(const ZigbeeUartAdapter &uartAdapter, m_adapterMonitor->availableAdapters()) {
        ZigbeeAdapter adapter = convertUartAdapterToAdapter(uartAdapter);
        qCDebug(dcZigbee()) << "Adapter added" << adapter;
        m_adapters.append(adapter);
    }

    connect(m_adapterMonitor, &ZigbeeUartAdapterMonitor::adapterAdded, this, [this](const ZigbeeUartAdapter &uartAdapter){
        ZigbeeAdapter adapter = convertUartAdapterToAdapter(uartAdapter);
        qCDebug(dcZigbee()) << "Adapter added" << adapter;
        m_adapters.append(adapter);
        emit availableAdapterAdded(adapter);
    });

    connect(m_adapterMonitor, &ZigbeeUartAdapterMonitor::adapterRemoved, this, [this](const ZigbeeUartAdapter &uartAdapter){
        foreach (const ZigbeeAdapter &adapter, m_adapters) {
            if (adapter.systemLocation() == uartAdapter.systemLocation()) {
                qCDebug(dcZigbee()) << "Adapter removed" << adapter;
                m_adapters.removeAll(adapter);
                emit availableAdapterRemoved(adapter);
            }
        }
    });

    // Load zigbee networks from settings
    loadZigbeeNetworks();

    // TODO: load platform configuration for networks we know for sure how they work
}

bool ZigbeeManager::available() const
{
    return !m_zigbeeNetworks.isEmpty();
}

bool ZigbeeManager::enabled() const
{
    return true;//m_zigbeeNetwork && m_zigbeeNetwork->state() != ZigbeeNetwork::StateUninitialized;
}

ZigbeeAdapters ZigbeeManager::availableAdapters() const
{
    return m_adapters;
}

QHash<QUuid, ZigbeeNetwork *> ZigbeeManager::zigbeeNetworks() const
{
    return m_zigbeeNetworks;
}

ZigbeeManager::ZigbeeError ZigbeeManager::createZigbeeNetwork(const ZigbeeAdapter &adapter, const ZigbeeChannelMask channelMask)
{
    qCDebug(dcZigbee()) << "Start creating network for" << adapter << channelMask;

    // Make sure we don't have aleardy a network for this adapter
    foreach (ZigbeeNetwork *existingNetwork, m_zigbeeNetworks.values()) {
        if (existingNetwork->serialPortName() == adapter.systemLocation()) {
            qCWarning(dcZigbee()) << "Failed to create a network for" << adapter << "because this adapter is already in use for network" << existingNetwork->networkUuid().toString();
            return ZigbeeManager::ZigbeeErrorAdapterAlreadyInUse;
        }
    }

    if (!m_adapters.contains(adapter)) {
        qCWarning(dcZigbee()) << "Failed to create a network for" << adapter << "because the adapter is not available any more";
        return ZigbeeManager::ZigbeeErrorAdapterNotAvailable;
    }

    ZigbeeNetwork *network = buildNetworkObject(QUuid::createUuid(), adapter.backendType());
    network->setChannelMask(channelMask);
    network->setSerialPortName(adapter.systemLocation());
    network->setSerialBaudrate(adapter.baudRate());
    addNetwork(network);

    qCDebug(dcZigbee()) << "Starting zigbee network" << network->networkUuid().toString();
    network->startNetwork();

    return ZigbeeErrorNoError;
}

ZigbeeManager::ZigbeeError ZigbeeManager::removeZigbeeNetwork(const QUuid &networkUuid)
{
    if (!m_zigbeeNetworks.keys().contains(networkUuid)) {
        qCWarning(dcZigbee()) << "Could not remove network with uuid" << networkUuid.toString() << "because there is no network with this uuid.";
        return ZigbeeManager::ZigbeeErrorNetworkUuidNotFound;
    }

    qCDebug(dcZigbee()) << "Removing network" << networkUuid.toString();
    ZigbeeNetwork *network = m_zigbeeNetworks.take(networkUuid);
    // Note: destroy will remove all nodes from the network and wipe/delete the database
    network->destroyNetwork();
    emit zigbeeNetworkRemoved(network->networkUuid());
    // Make sure to delete later, so all node removed signals can be processed
    network->deleteLater();

    // Delete network settings
    NymeaSettings settings(NymeaSettings::SettingsRoleZigbee);
    settings.beginGroup("ZigbeeNetworks");
    settings.beginGroup(network->networkUuid().toString());
    settings.remove("");
    settings.endGroup();
    settings.endGroup();

    qCDebug(dcZigbee()) << "Network removed successfully" << networkUuid.toString();
    return ZigbeeManager::ZigbeeErrorNoError;
}

ZigbeeManager::ZigbeeError ZigbeeManager::setZigbeeNetworkPermitJoin(const QUuid &networkUuid, quint16 shortAddress, int duration)
{
    if (!m_zigbeeNetworks.keys().contains(networkUuid)) {
        qCWarning(dcZigbee()) << "Could not set permit join network" << networkUuid.toString() << "because there is no network with this uuid.";
        return ZigbeeManager::ZigbeeErrorNetworkUuidNotFound;
    }

    ZigbeeNetwork *network = m_zigbeeNetworks.value(networkUuid);
    if (network->state() != ZigbeeNetwork::StateRunning) {
        qCWarning(dcZigbee()) << "Could not set permit join network" << networkUuid.toString() << "because the network is not running.";
        return ZigbeeManager::ZigbeeErrorNetworkOffline;
    }

    // TODO: set permit join

    qCDebug(dcZigbee()) << "Set permit join for network" << networkUuid.toString() << ZigbeeUtils::convertUint16ToHexString(shortAddress) << "to" << duration << "[s] successfully.";
    return ZigbeeManager::ZigbeeErrorNoError;
}

ZigbeeManager::ZigbeeError ZigbeeManager::factoryResetNetwork(const QUuid &networkUuid)
{
    if (!m_zigbeeNetworks.keys().contains(networkUuid)) {
        qCWarning(dcZigbee()) << "Could not factory reset network with uuid" << networkUuid.toString() << "because there is no network with this uuid.";
        return ZigbeeManager::ZigbeeErrorNetworkUuidNotFound;
    }

    ZigbeeNetwork *network = m_zigbeeNetworks.value(networkUuid);
    network->factoryResetNetwork();
    return ZigbeeManager::ZigbeeErrorNoError;
}

void ZigbeeManager::saveNetwork(ZigbeeNetwork *network)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleZigbee);
    settings.beginGroup("ZigbeeNetworks");
    settings.beginGroup(network->networkUuid().toString());
    settings.setValue("serialPort", network->serialPortName());
    settings.setValue("baudRate", network->serialBaudrate());
    switch (network->backendType()) {
    case Zigbee::ZigbeeBackendTypeDeconz:
        settings.setValue("backendType", static_cast<int>(ZigbeeAdapter::ZigbeeBackendTypeDeconz));
        break;
    case Zigbee::ZigbeeBackendTypeNxp:
        settings.setValue("backendType", static_cast<int>(ZigbeeAdapter::ZigbeeBackendTypeNxp));
        break;
    default:
        qCWarning(dcZigbee()) << "Unhandled backend type" << network->backendType() << "which is not implemented in nymea yet.";
        break;
    }
    settings.setValue("panId", network->panId());
    settings.setValue("channel", network->channel());
    settings.setValue("macAddress", network->macAddress().toString());
    settings.setValue("channelMask", network->channelMask().toUInt32());
    settings.setValue("networkKey", network->securityConfiguration().networkKey().toString());
    settings.setValue("trustCenterLinkKey", network->securityConfiguration().globalTrustCenterLinkKey().toString());

    settings.endGroup(); // networkUuid
    settings.endGroup(); // ZigbeeNetworks
}

void ZigbeeManager::loadZigbeeNetworks()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleZigbee);
    settings.beginGroup("ZigbeeNetworks");

    foreach (const QString networkUuidGroupString, settings.childGroups()) {
        settings.beginGroup(networkUuidGroupString);

        QUuid networkUuid = QUuid(networkUuidGroupString);
        QString serialPortName = settings.value("serialPort").toString();
        qint32 serialBaudRate = settings.value("baudRate").toInt();
        ZigbeeAdapter::ZigbeeBackendType backendType = static_cast<ZigbeeAdapter::ZigbeeBackendType>(settings.value("backendType").toInt());
        quint16 panId = static_cast<quint16>(settings.value("panId", 0).toUInt());
        quint8 channel = settings.value("channel", 0).toUInt();
        ZigbeeChannelMask channelMask = ZigbeeChannelMask(static_cast<quint32>(settings.value("channelMask").toUInt()));
        ZigbeeAddress macAddress = ZigbeeAddress(settings.value("macAddress").toString());
        ZigbeeSecurityConfiguration securityConfiguration;
        ZigbeeNetworkKey netKey(settings.value("networkKey", QString()).toString());
        if (netKey.isValid()) {
            securityConfiguration.setNetworkKey(netKey);
        }

        ZigbeeNetworkKey tcKey(settings.value("trustCenterLinkKey", QString("5A6967426565416C6C69616E63653039")).toString());
        if (!tcKey.isValid()) {
            securityConfiguration.setGlobalTrustCenterlinkKey(tcKey);
        }

        ZigbeeNetwork *network = buildNetworkObject(networkUuid, backendType);
        network->setSerialPortName(serialPortName);
        network->setSerialBaudrate(serialBaudRate);
        network->setMacAddress(macAddress);
        network->setPanId(panId);
        network->setChannel(channel);
        network->setChannelMask(channelMask);
        network->setSecurityConfiguration(securityConfiguration);
        addNetwork(network);
        settings.endGroup(); // networkUuid
    }
    settings.endGroup(); // ZigbeeNetworks

    if (m_zigbeeNetworks.isEmpty()) {
        qCDebug(dcZigbee()) << "There are no zigbee networks configured yet.";
    }

    // Start all loaded networks
    foreach (ZigbeeNetwork *network, m_zigbeeNetworks.values()) {
        network->startNetwork();
    }
}

ZigbeeNetwork *ZigbeeManager::buildNetworkObject(const QUuid &networkId, ZigbeeAdapter::ZigbeeBackendType backendType)
{
    ZigbeeNetwork *network = nullptr;
    switch (backendType) {
    case ZigbeeAdapter::ZigbeeBackendTypeDeconz:
        network = ZigbeeNetworkManager::createZigbeeNetwork(networkId, Zigbee::ZigbeeBackendTypeDeconz, this);
        break;
    case ZigbeeAdapter::ZigbeeBackendTypeNxp:
        network = ZigbeeNetworkManager::createZigbeeNetwork(networkId, Zigbee::ZigbeeBackendTypeNxp, this);
        break;
    }
    network->setSettingsDirectory(QDir(NymeaSettings::settingsPath()));
    return network;
}

void ZigbeeManager::addNetwork(ZigbeeNetwork *network)
{
    connect(network, &ZigbeeNetwork::stateChanged, this, [this, network](ZigbeeNetwork::State state){
        qCDebug(dcZigbee()) << "Network state changed" << network->networkUuid().toString() << state;

        // TODO: set state of zigbee resource depending on the state

        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::errorOccured, this, [network](ZigbeeNetwork::Error error){
        qCWarning(dcZigbee()) << "Network error occured for network" << network->networkUuid().toString() << error;

        // TODO: handle error
    });

    connect(network, &ZigbeeNetwork::panIdChanged, this, [this, network](quint16 panId){
        qCDebug(dcZigbee()) << "Network PAN ID changed" << network->networkUuid().toString() << panId;
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::channelChanged, this, [this, network](quint8 channel){
        qCDebug(dcZigbee()) << "Network channel changed" << network->networkUuid().toString() << channel;
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::macAddressChanged, this, [this, network](const ZigbeeAddress &macAddress){
        qCDebug(dcZigbee()) << "Network MAC address changed" << network->networkUuid().toString() << macAddress.toString();
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::securityConfigurationChanged, this, [this, network](const ZigbeeSecurityConfiguration &securityConfiguration){
        qCDebug(dcZigbee()) << "Network security configuration changed" << network->networkUuid().toString() << securityConfiguration.networkKey().toString();
        saveNetwork(network);
    });

    connect(network, &ZigbeeNetwork::channelMaskChanged, this, [this, network](const ZigbeeChannelMask &channelMask){
        qCDebug(dcZigbee()) << "Network channel mask changed" << network->networkUuid().toString() << channelMask;
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::permitJoiningChanged, this, [this, network](bool permitJoiningChanged){
        qCDebug(dcZigbee()) << "Network permit joining changed" << network->networkUuid().toString() << permitJoiningChanged;
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::nodeAdded, this, [network](ZigbeeNode *node){
        qCDebug(dcZigbee()) << "Network node added to network" << network->networkUuid().toString() << node;
    });

    connect(network, &ZigbeeNetwork::nodeRemoved, this, [network](ZigbeeNode *node){
        qCDebug(dcZigbee()) << "Network node removed from network" << network->networkUuid().toString() << node;
    });

    connect(network, &ZigbeeNetwork::firmwareVersionChanged, this, [this, network](const QString &firmwareVersion){
        qCDebug(dcZigbee()) << "Network adapter firmware version changed" << network->networkUuid().toString() << firmwareVersion;
        emit zigbeeNetworkChanged(network);
    });

    m_zigbeeNetworks.insert(network->networkUuid(), network);
    emit zigbeeNetworkAdded(network);
}

ZigbeeAdapter ZigbeeManager::convertUartAdapterToAdapter(const ZigbeeUartAdapter &uartAdapter)
{
    ZigbeeAdapter adapter;
    adapter.setName(uartAdapter.name());
    adapter.setSystemLocation(uartAdapter.systemLocation());
    adapter.setDescription(uartAdapter.description());
    adapter.setHardwareRecognized(uartAdapter.hardwareRecognized());
    adapter.setBaudRate(uartAdapter.baudRate());
    switch (uartAdapter.zigbeeBackend()) {
    case Zigbee::ZigbeeBackendTypeDeconz:
        adapter.setBackendType(ZigbeeAdapter::ZigbeeBackendTypeDeconz);
        break;
    case Zigbee::ZigbeeBackendTypeNxp:
        adapter.setBackendType(ZigbeeAdapter::ZigbeeBackendTypeNxp);
        break;
    default:
        qCWarning(dcZigbee()) << "Unhandled backend type" << uartAdapter.zigbeeBackend() << "which is not implemented in nymea yet.";
        break;
    }
    return adapter;
}

}
