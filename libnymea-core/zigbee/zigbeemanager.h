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

#ifndef ZIGBEEMANAGER_H
#define ZIGBEEMANAGER_H

#include <QObject>

#include <zigbeenetworkmanager.h>
#include <zigbeeuartadaptermonitor.h>

#include "zigbeeadapters.h"

namespace nymeaserver {

class ZigbeeManager : public QObject
{
    Q_OBJECT
public:
    enum ZigbeeNetworkState {
        ZigbeeNetworkStateOffline,
        ZigbeeNetworkStateStarting,
        ZigbeeNetworkStateUpdating,
        ZigbeeNetworkStateOnline,
        ZigbeeNetworkStateError
    };
    Q_ENUM(ZigbeeNetworkState)

    enum ZigbeeError {
        ZigbeeErrorNoError,
        ZigbeeErrorAdapterNotAvailable,
        ZigbeeErrorAdapterAlreadyInUse,
        ZigbeeErrorNetworkUuidNotFound,
        ZigbeeErrorDurationOutOfRange,
        ZigbeeErrorNetworkOffline,
        ZigbeeErrorUnknownBackend,
        ZigbeeErrorNodeNotFound,
        ZigbeeErrorForbidden,
        ZigbeeErrorInvalidChannel,
    };
    Q_ENUM(ZigbeeError)

    // Node information
    enum ZigbeeNodeType {
        ZigbeeNodeTypeCoordinator,
        ZigbeeNodeTypeRouter,
        ZigbeeNodeTypeEndDevice
    };
    Q_ENUM(ZigbeeNodeType)

    enum ZigbeeNodeState {
        ZigbeeNodeStateUninitialized,
        ZigbeeNodeStateInitializing,
        ZigbeeNodeStateInitialized,
        ZigbeeNodeStateHandled
    };
    Q_ENUM(ZigbeeNodeState)

    explicit ZigbeeManager(QObject *parent = nullptr);

    bool available() const;
    bool enabled() const;

    ZigbeeAdapters availableAdapters() const;
    QHash<QUuid, ZigbeeNetwork *> zigbeeNetworks() const;

    QPair<ZigbeeError, QUuid> createZigbeeNetwork(const QString &serialPort, uint baudRate, ZigbeeAdapter::ZigbeeBackendType backendType, ZigbeeChannelMask channelMask = ZigbeeChannelMask(ZigbeeChannelMask::ChannelConfigurationAllChannels));
    ZigbeeError removeZigbeeNetwork(const QUuid &networkUuid);
    ZigbeeError setZigbeeNetworkPermitJoin(const QUuid &networkUuid, quint16 shortAddress = Zigbee::BroadcastAddressAllRouters, uint duration = 120);
    ZigbeeError factoryResetNetwork(const QUuid &networkUuid);
    ZigbeeError refreshNeighborTables(const QUuid &networkUuid);

private:
    ZigbeeAdapters m_adapters;
    ZigbeeUartAdapterMonitor *m_adapterMonitor = nullptr;
    QHash<QUuid, ZigbeeNetwork *> m_zigbeeNetworks;

    bool m_available = false;
    bool m_autoSetupAdapters = false;

    void saveNetwork(ZigbeeNetwork *network);
    void loadZigbeeNetworks();
    void checkPlatformConfiguration();
    bool networkExistsForAdapter(const ZigbeeUartAdapter &uartAdapter);
    ZigbeeNetwork *createPlatformNetwork(const QString &serialPort, uint baudRate, Zigbee::ZigbeeBackendType backendType, ZigbeeChannelMask channelMask = ZigbeeChannelMask(ZigbeeChannelMask::ChannelConfigurationAllChannels));

    ZigbeeNetwork *buildNetworkObject(const QUuid &networkId, ZigbeeAdapter::ZigbeeBackendType backendType);
    void addNetwork(ZigbeeNetwork *network);

    ZigbeeAdapter convertUartAdapterToAdapter(const ZigbeeUartAdapter &uartAdapter);
    void evaluateZigbeeAvailable();
    void setupNodeSignals(ZigbeeNode *node);

signals:
    void availableChanged(bool available);

    void availableAdapterAdded(const ZigbeeAdapter &adapter);
    void availableAdapterRemoved(const ZigbeeAdapter &adapter);

    void zigbeeNetworkAdded(ZigbeeNetwork *zigbeeNetwork);
    void zigbeeNetworkRemoved(const QUuid networkUuid);
    void zigbeeNetworkChanged(ZigbeeNetwork *zigbeeNetwork);

    void nodeJoined(const QUuid &networkUuid, ZigbeeNode *node);
    void nodeAdded(const QUuid &networkUuid, ZigbeeNode *node);
    void nodeChanged(const QUuid &networkUuid, ZigbeeNode *node);
    void nodeRemoved(const QUuid &networkUuid, ZigbeeNode *node);
};

}

#endif // ZIGBEEMANAGER_H
