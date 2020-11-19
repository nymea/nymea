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
    qCDebug(dcZigbee()) << "Start creating network for" << serialPort << baudRate << backendType << channelMask;

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

    ZigbeeNetwork *network = buildNetworkObject(QUuid::createUuid(), backendType);
    network->setChannelMask(channelMask);
    network->setSerialPortName(serialPort);
    network->setSerialBaudrate(baudRate);
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
    settings.endGroup();
    settings.endGroup();

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

    qCDebug(dcZigbee()) << "Set permit joining in network" << network << "to" << duration << "seconds" << ZigbeeUtils::convertUint16ToHexString(shortAddress);
    network->setPermitJoining(duration, shortAddress);

    // Notify all clients about the new configuration
    emit zigbeeNetworkChanged(network);

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

    // FIXME: save also the serial number of the port if available.

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
     *
     * serialPort=/dev/ttyS0
     * baudRate=38400
     * backend=deconz
     */

    QFileInfo platformConfigurationFileInfo(NymeaSettings::settingsPath() + QDir::separator() + "zigbee-platform.conf");
    if (platformConfigurationFileInfo.exists()) {
        qCDebug(dcZigbee()) << "Found zigbee platform configuration" << platformConfigurationFileInfo.absoluteFilePath();
        QSettings platformSettings(platformConfigurationFileInfo.absoluteFilePath(), QSettings::IniFormat);
        QString serialPort = platformSettings.value("serialPort").toString();
        if (serialPort.isEmpty()) {
            qCWarning(dcZigbee()) << "The serial port is not specified correctly in the platform configuration file" << platformConfigurationFileInfo.absoluteFilePath() << "The platform based network will not be created.";
            return;
        }

        if (!m_adapterMonitor->hasAdapter(serialPort)) {
            qCWarning(dcZigbee()) << "Could not find platform specific serial port" << serialPort << "on this system. Please check the wiring or the port configuration. The platform based network will not be created.";
            return;
        }

        qint32 baudRate = platformSettings.value("baudRate").toUInt();
        QString backendString = platformSettings.value("backend").toString();
        Zigbee::ZigbeeBackendType backenType = Zigbee::ZigbeeBackendTypeNxp;
        if (backendString.toLower().contains("deconz")) {
            backenType = Zigbee::ZigbeeBackendTypeDeconz;
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
    }
    network->setSettingsDirectory(QDir(NymeaSettings::settingsPath()));
    return network;
}

void ZigbeeManager::addNetwork(ZigbeeNetwork *network)
{
    connect(network, &ZigbeeNetwork::stateChanged, this, [this, network](ZigbeeNetwork::State state){
        Q_UNUSED(state)
        qCDebug(dcZigbee()) << "Network state changed" << network;
        if (state == ZigbeeNetwork::StateRunning) {

            // Send a broadcast request to all powered nodes
            foreach (ZigbeeNode *node, network->nodes()) {
                if (node->macCapabilities().receiverOnWhenIdle && node->shortAddress() != 0x0000) {
                    node->deviceObject()->requestMgmtLqi();
                }
            }
        }

        evaluateZigbeeAvailable();
        emit zigbeeNetworkChanged(network);
    });

    connect(network, &ZigbeeNetwork::errorOccured, this, [this, network](ZigbeeNetwork::Error error){
        qCWarning(dcZigbee()) << "Network error occured for" << network << error;
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

    connect(network, &ZigbeeNetwork::nodeAdded, this, [this, network](ZigbeeNode *node){
        qCDebug(dcZigbee()) << "Node added to" << network << node;
        // The plugin don't need to see the coordinator node
        if (node->shortAddress() == 0) {
            return;
        }

        qCDebug(dcZigbee()) << "-->" << node;
        foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {
            qCDebug(dcZigbee()) << " " << endpoint;
            if (!endpoint->manufacturerName().isEmpty()) {
                qCDebug(dcZigbee()) << "  Manufacturer" << endpoint->manufacturerName();
                qCDebug(dcZigbee()) << "  Model" << endpoint->modelIdentifier();
                qCDebug(dcZigbee()) << "  Version" << endpoint->softwareBuildId();
            }
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


//        ZigbeeNodeInitializer *nodeInitializer = m_zigbeeNodeInitializers.value(network->networkUuid());
//        nodeInitializer->initializeNode(node);
        //TODO: emit node added once initialized so the plugins can use it

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

    connect(network, &ZigbeeNetwork::firmwareVersionChanged, this, [this, network](const QString &firmwareVersion){
        qCDebug(dcZigbee()) << "Network adapter firmware version changed for" << network << firmwareVersion;
        emit zigbeeNetworkChanged(network);
    });

    m_zigbeeNetworks.insert(network->networkUuid(), network);
    emit zigbeeNetworkAdded(network);

    // Create node initializer when a new node joins the network
    ZigbeeNodeInitializer *nodeInitializer = new ZigbeeNodeInitializer(network, this);
    connect(nodeInitializer, &ZigbeeNodeInitializer::nodeInitialized, this, [this, network](ZigbeeNode *node){
        qCDebug(dcZigbee()) << "Node initialied from nymea" << node;
        emit nodeAdded(network->networkUuid(), node);
    });

    qCDebug(dcZigbee()) << "Network added" << network;
    foreach (ZigbeeNode *node, network->nodes()) {
        qCDebug(dcZigbee()) << "-->" << node;
        foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {
            qCDebug(dcZigbee()) << " " << endpoint;
            if (!endpoint->manufacturerName().isEmpty()) {
                qCDebug(dcZigbee()) << "  Manufacturer" << endpoint->manufacturerName();
                qCDebug(dcZigbee()) << "  Model" << endpoint->modelIdentifier();
                qCDebug(dcZigbee()) << "  Version" << endpoint->softwareBuildId();
            }
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

    m_zigbeeNodeInitializers.insert(network->networkUuid(), nodeInitializer);
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
    default:
        qCWarning(dcZigbee()) << "Unhandled backend type" << uartAdapter.zigbeeBackend() << "which is not implemented in nymea yet.";
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

}
