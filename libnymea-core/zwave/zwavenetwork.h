/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#ifndef ZWAVENETWORK_H
#define ZWAVENETWORK_H

#include <QObject>
#include <QUuid>
#include <QHash>

#include "hardware/zwave/zwavenode.h"

namespace nymeaserver {
class ZWaveManager;
}

class ZWaveNetwork : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid networkUuid READ networkUuid CONSTANT)
    Q_PROPERTY(QString serialPort READ serialPort CONSTANT)
    Q_PROPERTY(ZWaveNetworkState networkSate READ networkState NOTIFY networkStateChanged)
    Q_PROPERTY(quint32 homeId READ homeId NOTIFY networkStateChanged)
    Q_PROPERTY(quint8 controllerNodeId READ controllerNodeId NOTIFY controllerNodeIdChanged)
    Q_PROPERTY(bool isZWavePlus READ isZWavePlus NOTIFY isZWavePlusChanged)
    Q_PROPERTY(bool isPrimaryController READ isPrimaryController NOTIFY isPrimaryControllerChanged)
    Q_PROPERTY(bool isStaticUpdateController READ isStaticUpdateController NOTIFY isStaticUpdateControllerChanged)
    Q_PROPERTY(bool isBridgeController READ isBridgeController NOTIFY isBridgeControllerChanged)
    Q_PROPERTY(bool waitingForNodeAddition READ waitingForNodeAddition NOTIFY waitingForNodeAdditionChanged)
    Q_PROPERTY(bool waitingForNodeRemoval READ waitingForNodeRemoval NOTIFY waitingForNodeRemovalChanged)

    friend class nymeaserver::ZWaveManager;

public:
    enum ZWaveNetworkState {
        ZWaveNetworkStateOffline,
        ZWaveNetworkStateStarting,
        ZWaveNetworkStateOnline,
        ZWaveNetworkStateError
    };
    Q_ENUM(ZWaveNetworkState)

    explicit ZWaveNetwork(const QUuid &networkUuid, const QString &serialPort, const QString &networkKey, QObject *parent = nullptr);

    QUuid networkUuid() const;
    QString serialPort() const;
    quint32 homeId() const;
    QString networkKey() const;
    quint8 controllerNodeId() const;
    ZWaveNetworkState networkState() const;
    bool isZWavePlus() const;
    bool isPrimaryController() const;
    bool isStaticUpdateController() const;
    bool isBridgeController() const;
    bool waitingForNodeAddition() const;
    bool waitingForNodeRemoval() const;

    ZWaveNodes nodes() const;
    ZWaveNode *node(quint8 nodeId) const;

signals:
    void networkStateChanged(ZWaveNetworkState state);
    void nodeAdded(ZWaveNode *node);
    void nodeRemoved(quint8 nodeId);
    void controllerNodeIdChanged(quint8 controllerNodeId);
    void isZWavePlusChanged(bool isZWavePlus);
    void isPrimaryControllerChanged(bool isPrimaryController);
    void isBridgeControllerChanged(bool isBridgeController);
    void isStaticUpdateControllerChanged(bool isStaticUpdateController);
    void waitingForNodeAdditionChanged(bool waitingForNodeAddition);
    void waitingForNodeRemovalChanged(bool waitingForNodeRemoval);

private:
    void addNode(ZWaveNode *node);
    void removeNode(quint8 nodeId);
    void setHomeId(quint32 homeId);
    void setControllerNodeId(quint8 controllerNodeId);
    void setIsZWavePlus(bool isZWavePlus);
    void setIsPrimaryController(bool isPrimaryController);
    void setIsStaticUpdateController(bool isStaticUpdateController);
    void setIsBridgeController(bool isBridgeController);
    void setNetworkState(ZWaveNetworkState networkState);
    void setWaitingForNodeAddition(bool waitingForNodeAddition);
    void setWaitingForNodeRemoval(bool waitingForNodeRemoval);

private:
    QUuid m_networkUuid;
    QString m_serialPort;
    quint32 m_homeId = 0;
    QString m_networkKey;
    quint8 m_controllerNodeId = 0;
    bool m_isZWavePlus = false;
    bool m_isPrimaryController = false;
    bool m_isStaticUpdateController = false;
    bool m_isBridgeController = false;
    bool m_waitingForNodeAddition = false;
    bool m_waitingForNodeRemoval = false;
    ZWaveNetworkState m_networkState = ZWaveNetworkStateOffline;

    QHash<quint8, ZWaveNode*> m_nodes;
};

class ZWaveNetworks: public QList<ZWaveNetwork*>
{
  Q_GADGET
public:
    ZWaveNetworks();
    ZWaveNetworks(const ZWaveNetworks &other);
    ZWaveNetworks(const QList<ZWaveNetwork *> &other);
};
Q_DECLARE_METATYPE(ZWaveNetworks)


#endif // ZWAVENETWORK_H
