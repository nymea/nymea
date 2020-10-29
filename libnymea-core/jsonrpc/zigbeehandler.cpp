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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zigbeehandler.h"
#include "zigbee/zigbeemanager.h"
#include "zigbee/zigbeeadapters.h"
#include "loggingcategories.h"

#include <zigbeeuartadapter.h>

namespace nymeaserver {

ZigbeeHandler::ZigbeeHandler(ZigbeeManager *zigbeeManager, QObject *parent) :
    JsonHandler(parent),
    m_zigbeeManager(zigbeeManager)
{
    qRegisterMetaType<nymeaserver::ZigbeeAdapter>();
    qRegisterMetaType<nymeaserver::ZigbeeAdapter::ZigbeeBackendType>();

    registerEnum<ZigbeeManager::ZigbeeNetworkState>();
    registerEnum<ZigbeeAdapter::ZigbeeBackendType>();
    registerEnum<ZigbeeManager::ZigbeeError>();

    registerObject<ZigbeeAdapter, ZigbeeAdapters>();

    // Network object describing a network instance
    QVariantMap zigbeeNetworkDescription;
    zigbeeNetworkDescription.insert("networkUuid", enumValueName(Uuid));
    zigbeeNetworkDescription.insert("serialPort", enumValueName(String));
    zigbeeNetworkDescription.insert("baudRate", enumValueName(Uint));
    zigbeeNetworkDescription.insert("macAddress", enumValueName(String));
    zigbeeNetworkDescription.insert("panId", enumValueName(Uint));
    zigbeeNetworkDescription.insert("channel", enumValueName(Uint));
    zigbeeNetworkDescription.insert("channelMask", enumValueName(Uint));
    zigbeeNetworkDescription.insert("permitJoin", enumValueName(Bool));
    zigbeeNetworkDescription.insert("backendType", enumRef<ZigbeeAdapter::ZigbeeBackendType>());
    zigbeeNetworkDescription.insert("networkState", enumRef<ZigbeeManager::ZigbeeNetworkState>());
    registerObject("ZigbeeNetwork", zigbeeNetworkDescription);

    QVariantMap params, returns;
    QString description;

    // GetAdapters
    params.clear(); returns.clear();
    description = "Get the list of available ZigBee adapter candidates in order to set up the zigbee network on the desired serial interface."
                  "If an adapter hardware has been recognized as a supported hardware, the \'hardwareRecognized\' property will be true and "
                  "the configurations can be used as they where given, otherwise the user might set the backend type and baud rate manually.";
    returns.insert("adapters", objectRef<ZigbeeAdapters>());
    registerMethod("GetAdapters", description, params, returns);

    // AdapterAdded notification
    params.clear();
    description = "Emitted whenever a new ZigBee adapter candidate has been detected in the system.";
    params.insert("adapter", objectRef<ZigbeeAdapter>());
    registerNotification("AdapterAdded", description, params);

    // AdapterRemoved notification
    params.clear();
    description = "Emitted whenever a ZigBee adapter has been removed from the system (i.e. unplugged).";
    params.insert("adapters", objectRef<ZigbeeAdapter>());
    registerNotification("AdapterRemoved", description, params);

    // GetNetworks
    params.clear(); returns.clear();
    description = "Returns the list of current configured zigbee networks.";
    returns.insert("zigbeeNetworks", QVariantList() << objectRef("ZigbeeNetwork"));
    registerMethod("GetNetworks", description, params, returns);

    // AddNetwork
    params.clear(); returns.clear();
    description = "Create a new zigbee network for the given zigbee adapter. The channel mask is optional and defaults to all channels. "
            "The quietest channel will be picked. The channel mask type is a TODO and will probably be a flag.";
    params.insert("adapter", objectRef<ZigbeeAdapter>());
    params.insert("o:channelMask", enumValueName(Uint));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("AddNetwork", description, params, returns);

    // RemoveNetwork
    params.clear(); returns.clear();
    description = "Remove the zigbee network with the given network uuid";
    params.insert("networkUuid", enumValueName(Uuid));
    returns.insert("zigbeeError", enumRef<ZigbeeManager::ZigbeeError>());
    registerMethod("RemoveNetwork", description, params, returns);

    // NetworkAdded notification
    params.clear();
    description = "Emitted whenever a new ZigBee network has been added.";
    params.insert("network", objectRef("ZigbeeNetwork"));
    registerNotification("NetworkAdded", description, params);

    // NetworkRemoved notification
    params.clear();
    description = "Emitted whenever a new ZigBee network has been removed.";
    params.insert("networkUuid", enumValueName(Uuid));
    registerNotification("NetworkRemoved", description, params);

    // NetworkChanged notification
    params.clear();
    description = "Emitted whenever a new ZigBee network has changed.";
    params.insert("network", objectRef("ZigbeeNetwork"));
    registerNotification("NetworkChanged", description, params);


    connect(m_zigbeeManager, &ZigbeeManager::availableAdapterAdded, this, [this](const ZigbeeAdapter &adapter){
        QVariantMap params;
        params.insert("adapter", pack(adapter));
        emit AdapterAdded(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::availableAdapterRemoved, this, [this](const ZigbeeAdapter &adapter){
        QVariantMap params;
        params.insert("adapter", pack(adapter));
        emit AdapterRemoved(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkAdded, this, [this](ZigbeeNetwork *network){
        QVariantMap params;
        params.insert("zigbeeNetwork", packNetwork(network));
        emit NetworkAdded(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkChanged, this, [this](ZigbeeNetwork *network){
        QVariantMap params;
        params.insert("zigbeeNetwork", packNetwork(network));
        emit NetworkChanged(params);
    });

    connect(m_zigbeeManager, &ZigbeeManager::zigbeeNetworkRemoved, this, [this](const QUuid &networkUuid){
        QVariantMap params;
        params.insert("networkUuid", networkUuid.toString());
        emit NetworkRemoved(params);
    });
}

QString ZigbeeHandler::name() const
{
    return "Zigbee";
}

JsonReply *ZigbeeHandler::GetAdapters(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList adapterList;
    foreach (const ZigbeeAdapter &adapter, m_zigbeeManager->availableAdapters()) {
        adapterList << pack(adapter);
    }
    returnMap.insert("adapters", adapterList);
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::AddNetwork(const QVariantMap &params)
{
    ZigbeeAdapter adapter = unpack<ZigbeeAdapter>(params.value("adapter"));
    ZigbeeManager::ZigbeeError error = m_zigbeeManager->createZigbeeNetwork(adapter);
    QVariantMap returnMap;
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(error));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::RemoveNetwork(const QVariantMap &params)
{
    QUuid networkUuid = params.value("networkUuid").toUuid();
    ZigbeeManager::ZigbeeError error = m_zigbeeManager->removeZigbeeNetwork(networkUuid);
    QVariantMap returnMap;
    returnMap.insert("zigbeeError", enumValueName<ZigbeeManager::ZigbeeError>(error));
    return createReply(returnMap);
}

JsonReply *ZigbeeHandler::GetNetworks(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returnMap;
    QVariantList networkList;
    foreach (ZigbeeNetwork *network, m_zigbeeManager->zigbeeNetworks().values()) {
        networkList.append(packNetwork(network));
    }
    returnMap.insert("zigbeeNetworks", networkList);
    return createReply(returnMap);
}

QVariantMap ZigbeeHandler::packNetwork(ZigbeeNetwork *network)
{
    QVariantMap networkMap;
    networkMap.insert("networkUuid", network->networkUuid());
    networkMap.insert("serialPort", network->serialPortName());
    networkMap.insert("baudRate", network->serialBaudrate());
    networkMap.insert("macAddress", network->macAddress().toString());
    networkMap.insert("panId", network->panId());
    networkMap.insert("channel", network->channel());
    networkMap.insert("channelMask", network->channelMask().toUInt32());
    networkMap.insert("permitJoin", network->permitJoining());
    switch (network->backendType()) {
    case Zigbee::ZigbeeBackendTypeDeconz:
        networkMap.insert("backendType", enumValueName<ZigbeeAdapter::ZigbeeBackendType>(ZigbeeAdapter::ZigbeeBackendTypeDeconz));
        break;
    case Zigbee::ZigbeeBackendTypeNxp:
        networkMap.insert("backendType", enumValueName<ZigbeeAdapter::ZigbeeBackendType>(ZigbeeAdapter::ZigbeeBackendTypeNxp));
        break;
    }

    switch (network->state()) {
    case ZigbeeNetwork::StateOffline:
    case ZigbeeNetwork::StateStopping:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateOffline));
        break;
    case ZigbeeNetwork::StateStarting:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateStarting));
        break;
    case ZigbeeNetwork::StateRunning:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateOnline));
        break;
    case ZigbeeNetwork::StateUpdating:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateUpdating));
        break;
    case ZigbeeNetwork::StateUninitialized:
        networkMap.insert("networkState", enumValueName<ZigbeeManager::ZigbeeNetworkState>(ZigbeeManager::ZigbeeNetworkStateError));
        break;
    }

    return networkMap;
}

}
