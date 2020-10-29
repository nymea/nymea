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

#include <zigbeeuartadapter.h>

namespace nymeaserver {

ZigbeeHandler::ZigbeeHandler(ZigbeeManager *zigbeeManager, QObject *parent) :
    JsonHandler(parent),
    m_zigbeeManager(zigbeeManager)
{
    registerEnum<ZigbeeManager::ZigbeeNetworkState>();
    registerEnum<ZigbeeAdapter::ZigbeeBackendType>();
    registerEnum<ZigbeeManager::ZigbeeError>();

    registerObject<ZigbeeAdapter, ZigbeeAdapters>();

    // Network object describing a network instance
    QVariantMap zigbeeNetworkDescription;
    zigbeeNetworkDescription.insert("uuid", enumValueName(Uuid));
    registerObject("ZigbeeNetwork", zigbeeNetworkDescription);

    QVariantMap params, returns;
    QString description;

    // GetAdapters
    params.clear(); returns.clear();
    description = "Get the list of available ZigBee adapter candidates in order to set up the zigbee network on the desired serial interface. If an adapter hardware has been recognized as a supported hardware, the \'hardwareRecognized\' property will be true and the configurations can be used as they where given, otherwise the user might set the backend type and baud rate manually.";
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

    params.clear(); returns.clear();
    description = "Get the status of the current network.";
    returns.insert("available", enumValueName(Bool));
    returns.insert("enabled", enumValueName(Bool));
    returns.insert("configured", enumValueName(Bool));
    returns.insert("serialPort", enumValueName(String));
    returns.insert("baudRate", enumValueName(Uint));
    returns.insert("backend", enumRef<Zigbee::ZigbeeBackendType>());
    returns.insert("firmwareVersion", enumValueName(String));
    returns.insert("networkIeeeeAddress", enumValueName(String));
    returns.insert("networkPanId", enumValueName(Uint));
    returns.insert("channel", enumValueName(Uint));
    returns.insert("permitJoin", enumValueName(Bool));
    returns.insert("networkState", enumRef<ZigbeeManager::ZigbeeNetworkState>());
    registerMethod("GetNetworkStatus", description, params, returns);

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

    /* 1. GetNetworks
     * 2. Setup network if the is no network configured
     *      - GetAdapters
     *      - AddNetwork (adapter, channelmask)
     *      -
     */

    // GetNetworkStatus
    // NetworkStatusChanged
    // SetupNetwork(network, uart, baudrate, backendtype)
    // PermitJoin(time, o:nwk address)
    // FactoryResetNetwork

    // GetNodes [Node(mac, nwkAddress, name, manufacturer, type, descriptors, associacted device, state, reachable)]
    // NodeAdded
    // NodeChanged (state, node base information, associated deviceId)
    // RemoveNode
    // NodeRemoved

    // GetGroups
    // GroupChanged
    // AddGroup
    // GroupAdded
    // RemoveGroup
    // GroupRemoved

}

QString ZigbeeHandler::name() const
{
    return "Zigbee";
}

JsonReply *ZigbeeHandler::GetNetworkStatus(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap ret;
//    if (m_zigbeeManager->zigbeeNetwork()) {
//        ret.insert("available", m_zigbeeManager->available());
//        ret.insert("enabled", m_zigbeeManager->enabled());
//        ret.insert("configured", true);
//        ret.insert("networkState", m_zigbeeManager->zigbeeNetwork()->state());
//    } else {
//        ret.insert("available", m_zigbeeManager->available());
//        ret.insert("enabled", m_zigbeeManager->enabled());
//        ret.insert("configured", false);
//        ret.insert("state", ZigbeeNetwork::StateUninitialized);
//    }

    return createReply(ret);
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

}
