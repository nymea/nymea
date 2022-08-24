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

#include <QDir>
#include <QFileInfo>

#include <zigbeeutils.h>

NYMEA_LOGGING_CATEGORY(dcZigbee, "Zigbee")

// Register debug category from the libnymea-zigbee
NYMEA_LOGGING_CATEGORY(dcZigbeeNetworkLibNymeaZigbee, "ZigbeeNetwork")

namespace nymeaserver {

ZigbeeManager::ZigbeeManager(QObject *parent) :
    QObject(parent)
{
    // Adapter monitor
    qCDebug(dcZigbee()) << "Initialize the Zigbee manager";
    m_adapterMonitor = new ZigbeeUartAdapterMonitor(this);
    if (!m_adapterMonitor->isValid()) {
        qCWarning(dcZigbee()) << "Could not initialize the Zigbee adapter monitor.";
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



        if (m_autoSetupAdapters) {
            if (networkExistsForAdapter(uartAdapter)) {
                qCDebug(dcZigbee()) << "Auto setup enabled. There is already a network configured for" << uartAdapter;
            } else {
                qCDebug(dcZigbee()) << "Auto setup enabled. No network for" << uartAdapter << "created yet. Creating network...";
                ZigbeeNetwork *network = createPlatformNetwork(uartAdapter.serialPort(), uartAdapter.baudRate(), uartAdapter.zigbeeBackend());
                if (!uartAdapter.serialNumber().isEmpty())
                    network->setSerialNumber(uartAdapter.serialNumber());

                addNetwork(network);
                network->startNetwork();
            }
        }

        // FIXME: check if serial number available and gets used by a network (adjust serial port if changed)

        emit availableAdapterAdded(adapter);
    });

    connect(m_adapterMonitor, &ZigbeeUartAdapterMonitor::adapterRemoved, this, [this](const ZigbeeUartAdapter &uartAdapter){
        foreach (const ZigbeeAdapter &adapter, m_adapters) {
            if (adapter.serialPort() == uartAdapter.serialPort()) {
                qCDebug(dcZigbee()) << "Adapter removed" << adapter;
                m_adapters.removeAll(adapter);
                emit availableAdapterRemoved(adapter);
            }
        }
    });

    // Load zigbee networks from settings
    loadZigbeeNetworks();

    // Check if we have a zigbee platform configuration and if we have to create the platform network automatically
    checkPlatformConfiguration();

    // Check if auto setup recognized adapters is enabled
    if (m_autoSetupAdapters) {
        qCDebug(dcZigbee()) << "Auto setup enabled. Verify existing networks and adapters...";
        foreach(const ZigbeeUartAdapter &uartAdapter, m_adapterMonitor->availableAdapters()) {
            if (networkExistsForAdapter(uartAdapter)) {
                qCDebug(dcZigbee()) << "Network for adapter" << uartAdapter << "already created.";
            } else {
                qCDebug(dcZigbee()) << "Auto setup enabled and there is no network for" << uartAdapter << "created yet. Creating network...";
                ZigbeeNetwork *network = createPlatformNetwork(uartAdapter.serialPort(), uartAdapter.baudRate(), uartAdapter.zigbeeBackend());
                if (!uartAdapter.serialNumber().isEmpty())
                    network->setSerialNumber(uartAdapter.serialNumber());

                addNetwork(network);
            }
        }
    }

    // Start all loaded networks
    foreach (ZigbeeNetwork *network, m_zigbeeNetworks.values()) {
        network->startNetwork();
    }
}

bool ZigbeeManager::available() const
{
    return m_available;
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

QPair<ZigbeeManager::ZigbeeError, QUuid> ZigbeeManager::createZigbeeNetwork(const QString &serialPort, uint baudRate, ZigbeeAdapter::ZigbeeBackendType backendType, ZigbeeChannelMask channelMask)
{
    qCDebug(dcZigbee()) << "Creating network for" << serialPort << baudRate << backendType << channelMask;

    // Make sure we don't have aleardy a network for this adapter
    foreach (ZigbeeNetwork *existingNetwork, m_zigbeeNetworks.values()) {
        if (existingNetwork->serialPortName() == serialPort) {
            qCWarning(dcZigbee()) << "Failed to create a network for" << serialPort << "because this adapter is already in use for network" << existingNetwork;
            return QPair<ZigbeeManager::ZigbeeError, QUuid>(ZigbeeManager::ZigbeeErrorAdapterAlreadyInUse, QUuid());
        }
    }

    if (!m_adapters.hasSerialPort(serialPort)) {
        qCWarning(dcZigbee()) << "Failed to create a network for" << serialPort << "because the adapter is not available any more";
        return QPair<ZigbeeManager::ZigbeeError, QUuid>(ZigbeeManager::ZigbeeErrorAdapterNotAvailable, QUuid());
    }

    QString serialNumber;
    foreach (const ZigbeeAdapter &adapter, m_adapters) {
        if (adapter.serialPort() == serialPort && !adapter.serialNumber().isEmpty()) {
            serialNumber = adapter.serialNumber();
        }
    }

    ZigbeeNetwork *network = buildNetworkObject(QUuid::createUuid(), backendType);
    network->setChannelMask(channelMask);
    network->setSerialPortName(serialPort);
    network->setSerialBaudrate(baudRate);
    network->setSerialNumber(serialNumber);
    addNetwork(network);

    qCDebug(dcZigbee()) << "Starting" << network;
    network->startNetwork();
    return QPair<ZigbeeManager::ZigbeeError, QUuid>(ZigbeeManager::ZigbeeErrorNoError, network->networkUuid());
}

ZigbeeManager::ZigbeeError ZigbeeManager::removeZigbeeNetwork(const QUuid &networkUuid)
{
    if (!m_zigbeeNetworks.keys().contains(networkUuid)) {
        qCWarning(dcZigbee()) << "Could not remove network with uuid" << networkUuid.toString() << "because there is no network with this uuid.";
        return ZigbeeManager::ZigbeeErrorNetworkUuidNotFound;
    }

    ZigbeeNetwork *network = m_zigbeeNetworks.value(networkUuid);
    qCDebug(dcZigbee()) << "Removing" << network;
    // Note: destroy will remove all nodes from the network and wipe/delete the database
    network->destroyNetwork();
    emit zigbeeNetworkRemoved(network->networkUuid());

    // Make sure to delete later, so all node removed signals can be processed
    m_zigbeeNetworks.remove(networkUuid);
    network->deleteLater();

    // Delete network settings
    NymeaSettings settings(NymeaSettings::SettingsRoleZigbee);
    settings.beginGroup("ZigbeeNetworks");
    settings.beginGroup(network->networkUuid().toString());
    settings.remove("");
    settings.endGroup(); // networkUuid
    settings.endGroup(); // ZigbeeNetworks

    qCDebug(dcZigbee()) << "Network removed successfully";
    return ZigbeeManager::ZigbeeErrorNoError;
}

ZigbeeManager::ZigbeeError ZigbeeManager::setZigbeeNetworkPermitJoin(const QUuid &networkUuid, quint16 shortAddress, uint duration)
{
    if (!m_zigbeeNetworks.keys().contains(networkUuid)) {
        qCWarning(dcZigbee()) << "Could not set permit join network" << networkUuid.toString() << "because there is no network with this uuid.";
        return ZigbeeManager::ZigbeeErrorNetworkUuidNotFound;
    }

    if (duration > 255) {
        qCWarning(dcZigbee()) << "The given duration for permit join is out of range. Only values between 0 and 255 are allowed.";
        return ZigbeeManager::ZigbeeErrorDurationOutOfRange;
    }

    ZigbeeNetwork *network = m_zigbeeNetworks.value(networkUuid);
    if (network->state() != ZigbeeNetwork::StateRunning) {
        qCWarning(dcZigbee()) << "Could not set permit join in" << network << "because the network is not running.";
        return ZigbeeManager::ZigbeeErrorNetworkOffline;
    }

    // FIXME: figure out why unicast permit join is not working so far...
    if (network->hasNode(shortAddress)) {
        ZigbeeNode *node = network->getZigbeeNode(shortAddress);
        // We have a short address of a node, lets unicast the request
        qCDebug(dcZigbee()) << "Set permit joining" << node << "to" << duration << "seconds" << ZigbeeUtils::convertUint16ToHexString(shortAddress);
        node->deviceObject()->requestMgmtPermitJoining(duration);
    } else {
        // Probably a broadcast address, let the network handle this
        qCDebug(dcZigbee()) << "Set permit joining in network" << network << "to" << duration << "seconds" << ZigbeeUtils::convertUint16ToHexString(shortAddress);
        network->setPermitJoining(duration, shortAddress);
    }

    return ZigbeeManager::ZigbeeErrorNoError;
}

ZigbeeManager::ZigbeeError ZigbeeManager::factoryResetNetwork(const QUuid &networkUuid)
{
    if (!m_zigbeeNetworks.keys().contains(networkUuid)) {
        qCWarning(dcZigbee()) << "Could not factory reset network with uuid" << networkUuid.toString() << "because there is no network with this uuid.";
        return ZigbeeManager::ZigbeeErrorNetworkUuidNotFound;
    }

    ZigbeeNetwork *network = m_zigbeeNetworks.value(networkUuid);
    qCDebug(dcZigbee()) << "Start factory resetting" << network;
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
    case Zigbee::ZigbeeBackendTypeTi:
        settings.setValue("backendType", static_cast<int>(ZigbeeAdapter::ZigbeeBackendTypeTi));
        break;
    }
    settings.setValue("panId", network->panId());
    settings.setValue("channel", network->channel());
    settings.setValue("macAddress", network->macAddress().toString());
    settings.setValue("channelMask", network->channelMask().toUInt32());
    settings.setValue("networkKey", network->securityConfiguration().networkKey().toString());
    settings.setValue("trustCenterLinkKey", network->securityConfiguration().globalTrustCenterLinkKey().toString());
    if (!network->serialNumber().isEmpty()) {
        settings.setValue("serialNumber", network->serialNumber());
    }

    settings.endGroup(); // networkUuid
    settings.endGroup(); // ZigbeeNetworks
}

void ZigbeeManager::loadZigbeeNetworks()
{
    NymeaSettings settings(NymeaSettings::SettingsRoleZigbee);
    qCDebug(dcZigbee()) << "Loading zigbee networks from" << settings.fileName();
    settings.beginGroup("ZigbeeNetworks");
    foreach (const QString networkUuidGroupString, settings.childGroups()) {
        settings.beginGroup(networkUuidGroupString);

        // FIXME: load also the serial number of the port if available and search that serial port

        QUuid networkUuid = QUuid(networkUuidGroupString);
        QString serialPortName = settings.value("serialPort").toString();
        QString serialNumber = settings.value("serialNumber").toString();
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
        if (!network) {
            qCWarning(dcZigbee()) << "Invalid zigbee network configuration. Skipping network" << networkUuid;
            continue;
        }
        network->setSerialPortName(serialPortName);
        network->setSerialBaudrate(serialBaudRate);
        network->setSerialNumber(serialNumber);
        network->setMacAddress(macAddress);
        network->setPanId(panId);
        network->setChannel(channel);
        network->setChannelMask(channelMask);
        network->setSecurityConfiguration(securityConfiguration);
        settings.endGroup(); // networkUuid

        // Load the nodes before adding the network internally, so the loaded nodes will not generate added signals
        network->loadNetwork();

        // Add the network internally
        addNetwork(network);
    }
    settings.endGroup(); // ZigbeeNetworks

    if (m_zigbeeNetworks.isEmpty()) {
        qCDebug(dcZigbee()) << "There are no zigbee networks configured yet.";
        return;
    }

}

void ZigbeeManager::checkPlatformConfiguration()
{
    /* Example platform configurations
     *
     * serialPort=/dev/ttymxc2
     * baudRate=115200
     * backend=nxp
     * autoSetup=false
     *
     * serialPort=/dev/ttyS0
     * baudRate=38400
     * backend=deconz
     * autoSetup=false
     *
     * autoSetup=true
     */

    QFileInfo platformConfigurationFileInfo(NymeaSettings::settingsPath() + QDir::separator() + "zigbee-platform.conf");
    if (platformConfigurationFileInfo.exists()) {
        qCDebug(dcZigbee()) << "Found zigbee platform configuration" << platformConfigurationFileInfo.absoluteFilePath();
        QSettings platformSettings(platformConfigurationFileInfo.absoluteFilePath(), QSettings::IniFormat);
        QString serialPort = platformSettings.value("serialPort").toString();
        qint32 baudRate = platformSettings.value("baudRate").toUInt();
        QString backendString = platformSettings.value("backend").toString();
        m_autoSetupAdapters = platformSettings.value("autoSetup").toBool();
        Zigbee::ZigbeeBackendType backenType = Zigbee::ZigbeeBackendTypeNxp;
        if (backendString.toLower().contains("deconz")) {
            backenType = Zigbee::ZigbeeBackendTypeDeconz;
        }

        if (serialPort.isEmpty()) {
            qCWarning(dcZigbee()) << "The serial port is not specified correctly in the platform configuration file" << platformConfigurationFileInfo.absoluteFilePath() << "The platform based network will not be created.";
            return;
        }

        if (!m_adapterMonitor->hasAdapter(serialPort)) {
            qCWarning(dcZigbee()) << "Could not find platform specific serial port" << serialPort << "on this system. Please check the wiring or the port configuration. The platform based network will not be created.";
            return;
        }

        bool alreadyCreated = false;
        foreach (ZigbeeNetwork *network, m_zigbeeNetworks.values()) {
            if (network->serialPortName() == serialPort && network->serialBaudrate() == baudRate && network->backendType() == backenType) {
                qCDebug(dcZigbee()) << "Network based on platform configuration already created" << network;
                alreadyCreated = true;
                break;
            }
        }

        if (!alreadyCreated) {
            qCDebug(dcZigbee()) << "Network based on platform configuration has not been created yet.";
            ZigbeeNetwork *network = createPlatformNetwork(serialPort, baudRate, backenType);
            addNetwork(network);
            // Note: it will be saved once the network has started successfully
        }
    } else {
        qCDebug(dcZigbee()) << "No platform configuration specified.";
    }
}

bool ZigbeeManager::networkExistsForAdapter(const ZigbeeUartAdapter &uartAdapter)
{
    bool networkCreated = false;
    foreach (ZigbeeNetwork *network, m_zigbeeNetworks) {
        // Use the serial number if possible as refference
        if (!uartAdapter.serialNumber().isEmpty()) {
            if (network->serialNumber() == uartAdapter.serialNumber()) {
                networkCreated = true;
                break;
            }
        }

        // Otherwise check the serial port
        if (network->serialPortName() == uartAdapter.serialPort()) {
            networkCreated = true;
            break;
        }
    }
    return networkCreated;
}

ZigbeeNetwork *ZigbeeManager::createPlatformNetwork(const QString &serialPort, uint baudRate, Zigbee::ZigbeeBackendType backendType, ZigbeeChannelMask channelMask)
{
    qCDebug(dcZigbee()) << "Creating platform network on" << serialPort << baudRate << backendType << channelMask;
    ZigbeeNetwork *network = ZigbeeNetworkManager::createZigbeeNetwork(QUuid::createUuid(), backendType);
    network->setSettingsDirectory(QDir(NymeaSettings::settingsPath()));
    network->setChannelMask(channelMask);
    network->setSerialPortName(serialPort);
    network->setSerialBaudrate(baudRate);
    return network;
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
    case ZigbeeAdapter::ZigbeeBackendTypeTi:
        network = ZigbeeNetworkManager::createZigbeeNetwork(networkId, Zigbee::ZigbeeBackendTypeTi, this);
        break;
    }
    if (network) {
        network->setSettingsDirectory(QDir(NymeaSettings::settingsPath()));
    }
    return network;
}

void ZigbeeManager::addNetwork(ZigbeeNetwork *network)
{
    connect(network, &ZigbeeNetwork::stateChanged, this, [this, network](ZigbeeNetwork::State state){
        Q_UNUSED(state)
        qCDebug(dcZigbee()) << "Network state changed" << network;
        evaluateZigbeeAvailable();
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::errorOccured, this, [this, network](ZigbeeNetwork::Error error){
        qCWarning(dcZigbee()) << "Network error occurred for" << network << error;
        evaluateZigbeeAvailable();
        // TODO: handle error
    });

    connect(network, &ZigbeeNetwork::panIdChanged, this, [this, network](quint16 panId){
        qCDebug(dcZigbee()) << "Network PAN ID changed for" << network << panId;
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::channelChanged, this, [this, network](quint8 channel){
        qCDebug(dcZigbee()) << "Network channel changed for" << network << channel;
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::macAddressChanged, this, [this, network](const ZigbeeAddress &macAddress){
        qCDebug(dcZigbee()) << "Network MAC address changed for" << network << macAddress.toString();
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::securityConfigurationChanged, this, [this, network](const ZigbeeSecurityConfiguration &securityConfiguration){
        qCDebug(dcZigbee()) << "Network security configuration changed for" << network << securityConfiguration.networkKey().toString() << securityConfiguration.globalTrustCenterLinkKey().toString();
        saveNetwork(network);
    });

    connect(network, &ZigbeeNetwork::channelMaskChanged, this, [this, network](const ZigbeeChannelMask &channelMask){
        qCDebug(dcZigbee()) << "Network channel mask changed for" << network << channelMask;
        saveNetwork(network);
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::permitJoiningEnabledChanged, this, [this, network](bool permitJoiningEnabled){
        qCDebug(dcZigbee()) << "Network permit joining changed" << network->networkUuid().toString() << permitJoiningEnabled;
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::permitJoinDurationChanged, this, [this, network](bool permitJoinDuration){
        qCDebug(dcZigbee()) << "Network permit join duration changed" << network->networkUuid().toString() << permitJoinDuration;
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::permitJoinRemainingChanged, this, [this, network](bool permitJoinRemaining){
        qCDebug(dcZigbee()) << "Network permit join remaining changed" << network->networkUuid().toString() << permitJoinRemaining;
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::nodeAdded, this, [this, network](ZigbeeNode *node){
        qCDebug(dcZigbee()) << "Node added to" << network << node;
        // The plugin don't need to see the coordinator node
        if (node->shortAddress() == 0) {
            return;
        }

        qCDebug(dcZigbee()) << "-->" << node;
        foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {
            qCDebug(dcZigbee()) << " " << endpoint;
            if (!endpoint->manufacturerName().isEmpty())
                qCDebug(dcZigbee()) << "  Manufacturer:" << endpoint->manufacturerName();

            if (!endpoint->modelIdentifier().isEmpty())
                qCDebug(dcZigbee()) << "  Model" << endpoint->modelIdentifier();

            if (!endpoint->softwareBuildId().isEmpty())
                qCDebug(dcZigbee()) << "  Version" << endpoint->softwareBuildId();

            qCDebug(dcZigbee()) << "    Input clusters (" << endpoint->inputClusters().count() << ")";
            foreach (ZigbeeCluster *cluster, endpoint->inputClusters()) {
                qCDebug(dcZigbee()) << "     -" << cluster;
                foreach(const ZigbeeClusterAttribute &attribute, cluster->attributes()) {
                    qCDebug(dcZigbee()) << "       - " << attribute;
                }
            }

            qCDebug(dcZigbee()) << "    Output clusters (" << endpoint->outputClusters().count() << ")";
            foreach (ZigbeeCluster *cluster, endpoint->outputClusters()) {
                qCDebug(dcZigbee()) << "     -" << cluster;
                foreach(const ZigbeeClusterAttribute &attribute, cluster->attributes()) {
                    qCDebug(dcZigbee()) << "       - " << attribute;
                }
            }
        }

        emit nodeAdded(network->networkUuid(), node);
    });

    connect(network, &ZigbeeNetwork::nodeRemoved, this, [this, network](ZigbeeNode *node){
        qCDebug(dcZigbee()) << "Node removed from" << network->networkUuid().toString() << node;
        // The plugin don't need to see the coordinator node
        if (node->shortAddress() == 0) {
            return;
        }
        emit nodeRemoved(network->networkUuid(), node);
    });

    connect(network, &ZigbeeNetwork::nodeJoined, this, [this, network](ZigbeeNode *node){
        // The node has joined but is still initializing. Once the node has been initialized by the stack, the node added signal will be emitted.
        qCDebug(dcZigbee()) << "Node joined the network" << network->networkUuid().toString() << node;

        emit nodeJoined(network->networkUuid(), node);
        setupNodeSignals(node);
    });

    connect(network, &ZigbeeNetwork::firmwareVersionChanged, this, [this, network](const QString &firmwareVersion){
        qCDebug(dcZigbee()) << "Network adapter firmware version changed for" << network << firmwareVersion;
        emit zigbeeNetworkChanged(network);
    });

    m_zigbeeNetworks.insert(network->networkUuid(), network);
    emit zigbeeNetworkAdded(network);

    qCDebug(dcZigbee()) << "Network added" << network;
    foreach (ZigbeeNode *node, network->nodes()) {
        qCDebug(dcZigbee()) << "-->" << node;
        foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {
            qCDebug(dcZigbee()) << " " << endpoint;
            if (!endpoint->manufacturerName().isEmpty())
                qCDebug(dcZigbee()) << "  Manufacturer:" << endpoint->manufacturerName();

            if (!endpoint->modelIdentifier().isEmpty())
                qCDebug(dcZigbee()) << "  Model" << endpoint->modelIdentifier();

            if (!endpoint->softwareBuildId().isEmpty())
                qCDebug(dcZigbee()) << "  Version" << endpoint->softwareBuildId();

            qCDebug(dcZigbee()) << "    Input clusters (" << endpoint->inputClusters().count() << ")";
            foreach (ZigbeeCluster *cluster, endpoint->inputClusters()) {
                qCDebug(dcZigbee()) << "     -" << cluster;
                foreach(const ZigbeeClusterAttribute &attribute, cluster->attributes()) {
                    qCDebug(dcZigbee()) << "       - " << attribute;
                }
            }

            qCDebug(dcZigbee()) << "    Output clusters (" << endpoint->outputClusters().count() << ")";
            foreach (ZigbeeCluster *cluster, endpoint->outputClusters()) {
                qCDebug(dcZigbee()) << "     -" << cluster;
                foreach(const ZigbeeClusterAttribute &attribute, cluster->attributes()) {
                    qCDebug(dcZigbee()) << "       - " << attribute;
                }
            }
        }
    }

    foreach (ZigbeeNode *node, network->nodes()) {
        setupNodeSignals(node);
    }
}

ZigbeeAdapter ZigbeeManager::convertUartAdapterToAdapter(const ZigbeeUartAdapter &uartAdapter)
{
    ZigbeeAdapter adapter;
    adapter.setName(uartAdapter.name());
    adapter.setSerialPort(uartAdapter.serialPort());
    adapter.setDescription(uartAdapter.description());
    adapter.setSerialNumber(uartAdapter.serialNumber());
    adapter.setHardwareRecognized(uartAdapter.hardwareRecognized());
    adapter.setBaudRate(uartAdapter.baudRate());
    switch (uartAdapter.zigbeeBackend()) {
    case Zigbee::ZigbeeBackendTypeDeconz:
        adapter.setBackendType(ZigbeeAdapter::ZigbeeBackendTypeDeconz);
        break;
    case Zigbee::ZigbeeBackendTypeNxp:
        adapter.setBackendType(ZigbeeAdapter::ZigbeeBackendTypeNxp);
        break;
    case Zigbee::ZigbeeBackendTypeTi:
        adapter.setBackendType(ZigbeeAdapter::ZigbeeBackendTypeTi);
        break;
    }
    return adapter;
}

void ZigbeeManager::evaluateZigbeeAvailable()
{
    bool zigbeeAvailable = false;
    if (!m_zigbeeNetworks.isEmpty()) {
        foreach (ZigbeeNetwork *network, m_zigbeeNetworks.values()) {
            if (network->state() == ZigbeeNetwork::StateRunning) {
                zigbeeAvailable = true;
                break;
            }
        }
    }

    if (m_available == zigbeeAvailable)
        return;

    qCDebug(dcZigbee()) << "Zigbee is" << (zigbeeAvailable ? "now available" : "not available any more.");
    m_available = zigbeeAvailable;
    emit availableChanged(m_available);
}

void ZigbeeManager::setupNodeSignals(ZigbeeNode *node)
{
    // Connect signals while joining for initializing

    connect(node, &ZigbeeNode::shortAddressChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::stateChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::manufacturerNameChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::modelNameChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::versionChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::lqiChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::lastSeenChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });

    connect(node, &ZigbeeNode::reachableChanged, this, [=](){
        emit nodeChanged(node->networkUuid(), node);
    });
}


}
