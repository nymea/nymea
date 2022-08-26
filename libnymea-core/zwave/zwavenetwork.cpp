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

#include "zwavenetwork.h"

ZWaveNetwork::ZWaveNetwork(const QUuid &networkUuid, const QString &serialPort, const QString &networkKey, QObject *parent):
    QObject(parent),
    m_networkUuid(networkUuid),
    m_serialPort(serialPort),
    m_networkKey(networkKey)
{

}

QUuid ZWaveNetwork::networkUuid() const
{
    return m_networkUuid;
}

QString ZWaveNetwork::serialPort() const
{
    return m_serialPort;
}

ZWaveNetwork::ZWaveNetworkState ZWaveNetwork::networkState() const
{
    return m_networkState;
}

quint32 ZWaveNetwork::homeId() const
{
    return m_homeId;
}

QString ZWaveNetwork::networkKey() const
{
    return m_networkKey;
}

quint8 ZWaveNetwork::controllerNodeId() const
{
    return m_controllerNodeId;
}

ZWaveNodes ZWaveNetwork::nodes() const
{
    return m_nodes.values();
}

void ZWaveNetwork::setHomeId(quint32 homeId)
{
    m_homeId = homeId;
}

void ZWaveNetwork::setControllerNodeId(quint8 controllerNodeId)
{
    if (m_controllerNodeId != controllerNodeId) {
        m_controllerNodeId = controllerNodeId;
        emit controllerNodeIdChanged(controllerNodeId);
    }
}

bool ZWaveNetwork::isZWavePlus() const
{
    return m_isZWavePlus;
}

void ZWaveNetwork::setIsZWavePlus(bool isZWavePlus)
{
    if (m_isZWavePlus != isZWavePlus) {
        m_isZWavePlus = isZWavePlus;
        isZWavePlusChanged(isZWavePlus);
    }
}

bool ZWaveNetwork::isPrimaryController() const
{
    return m_isPrimaryController;
}

void ZWaveNetwork::setIsPrimaryController(bool isPrimaryController)
{
    if (m_isPrimaryController != isPrimaryController) {
        m_isPrimaryController = isPrimaryController;
        emit isPrimaryControllerChanged(isPrimaryController);
    }
}

bool ZWaveNetwork::isStaticUpdateController() const
{
    return m_isStaticUpdateController;
}

bool ZWaveNetwork::waitingForNodeAddition() const
{
    return m_waitingForNodeAddition;
}

bool ZWaveNetwork::waitingForNodeRemoval() const
{
    return m_waitingForNodeRemoval;
}

void ZWaveNetwork::setIsStaticUpdateController(bool isStaticUpdateController)
{
    if (m_isStaticUpdateController != isStaticUpdateController) {
        m_isStaticUpdateController = isStaticUpdateController;
        emit isStaticUpdateControllerChanged(isStaticUpdateController);
    }
}

bool ZWaveNetwork::isBridgeController() const
{
    return m_isBridgeController;
}

void ZWaveNetwork::setIsBridgeController(bool isBridgeController)
{
    if (m_isBridgeController != isBridgeController) {
        m_isBridgeController = isBridgeController;
        emit isBridgeControllerChanged(isBridgeController);
    }
}

ZWaveNode *ZWaveNetwork::node(quint8 nodeId) const
{
    return m_nodes.value(nodeId);
}

void ZWaveNetwork::addNode(ZWaveNode *node)
{
    node->setParent(this);
    m_nodes.insert(node->nodeId(), node);
    emit nodeAdded(node);
}

void ZWaveNetwork::removeNode(quint8 nodeId)
{
    m_nodes.take(nodeId)->deleteLater();
    emit nodeRemoved(nodeId);
}

void ZWaveNetwork::setNetworkState(ZWaveNetworkState networkState)
{
    if (m_networkState != networkState) {
        m_networkState = networkState;
        emit networkStateChanged(m_networkState);
    }
}

void ZWaveNetwork::setWaitingForNodeAddition(bool waitingForNodeAddition)
{
    if (m_waitingForNodeAddition != waitingForNodeAddition) {
        m_waitingForNodeAddition = waitingForNodeAddition;
        emit waitingForNodeAdditionChanged(waitingForNodeAddition);
    }
}

void ZWaveNetwork::setWaitingForNodeRemoval(bool waitingForNodeRemoval)
{
    if (m_waitingForNodeRemoval != waitingForNodeRemoval) {
        m_waitingForNodeRemoval = waitingForNodeRemoval;
        emit waitingForNodeRemovalChanged(waitingForNodeRemoval);
    }
}


ZWaveNetworks::ZWaveNetworks()
{

}

ZWaveNetworks::ZWaveNetworks(const ZWaveNetworks &other):
    QList<ZWaveNetwork*>(other)
{

}

ZWaveNetworks::ZWaveNetworks(const QList<ZWaveNetwork *> &other):
    QList<ZWaveNetwork*>(other)
{

}
