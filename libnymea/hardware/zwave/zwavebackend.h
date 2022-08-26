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

#ifndef ZWAVEBACKEND_H
#define ZWAVEBACKEND_H

#include "zwave.h"
#include "zwavereply.h"
#include "zwavenode.h"
#include "zwavevalue.h"

#include <QObject>
#include <QUuid>

class ZWaveBackend : public QObject
{
    Q_OBJECT
public:
    explicit ZWaveBackend(QObject *parent = nullptr);
    virtual ~ZWaveBackend() = default;

    virtual bool startNetwork(const QUuid &networkUuid, const QString &serialPort, const QString &networkKey = QString()) = 0;
    virtual bool stopNetwork(const QUuid &networkUuid) = 0;

    virtual quint32 homeId(const QUuid &networkUuid) = 0;
    virtual quint8 controllerNodeId(const QUuid &networkUuid) = 0;
    virtual bool isPrimaryController(const QUuid &networkUuid) = 0;
    virtual bool isStaticUpdateController(const QUuid &networkUuid) = 0;
    virtual bool isBridgeController(const QUuid &networkUuid) = 0;

    virtual bool factoryResetNetwork(const QUuid &networkUuid) = 0;

    virtual ZWaveReply* addNode(const QUuid &networkUuid, bool useSecurity) = 0;
    virtual ZWaveReply* removeNode(const QUuid &networkUuid) = 0;
    virtual ZWaveReply* removeFailedNode(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual ZWaveReply* cancelPendingOperation(const QUuid &networkUuid) = 0;

    virtual bool isNodeAwake(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual bool isNodeFailed(const QUuid &networkUuid, quint8 nodeId) = 0;

    virtual QString nodeName(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual ZWaveNode::ZWaveNodeType nodeType(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual ZWaveNode::ZWaveDeviceType nodeDeviceType(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual ZWaveNode::ZWaveNodeRole nodeRole(const QUuid &networkUiid, quint8 nodeId) = 0;
    virtual quint8 nodeSecurityMode(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual quint16 nodeManufacturerId(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual QString nodeManufacturerName(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual quint16 nodeProductId(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual QString nodeProductName(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual quint16 nodeProductType(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual quint8 nodeVersion(const QUuid &networkUuid, quint8 nodeId) = 0;

    virtual bool nodeIsZWavePlus(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual ZWaveNode::ZWavePlusDeviceType nodePlusDeviceType(const QUuid &networkUuid, quint8 nodeId) = 0;

    virtual bool nodeIsBeamingDevice(const QUuid &networkUuid, quint8 nodeId) = 0;
    virtual bool nodeIsSecureDevice(const QUuid &networkUuid, quint8 nodeId) = 0;

    virtual bool setValue(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value) = 0;

signals:
    void networkStarted(const QUuid &networkUuid);
    void networkFailed(const QUuid &networkUuid);
    void waitingForNodeAdditionChanged(const QUuid &networkUuid, bool waitingForNodeAddition);
    void waitingForNodeRemovalChanged(const QUuid &networkUuid, bool waitingForNodeRemoval);

    void nodeAdded(const QUuid &networkUuid, quint8 nodeId);
    void nodeRemoved(const QUuid &networkUuid, quint8 nodeId);
    void nodeInitialized(const QUuid &networkUuid, quint8 nodeId);

    void nodeDataChanged(const QUuid &networkUuid, quint8 nodeId);

    void nodeReachableStatus(const QUuid &networkUuid, quint8 nodeId, bool reachable);
    void nodeFailedStatus(const QUuid &networkUuid, quint8 nodeId, bool failed);
    void nodeSleepStatus(const QUuid &networkUuid, quint8 nodeId, bool sleeping);
    void nodeLinkQualityStatus(const QUuid &networkUuid, quint8 nodeId, quint8 linkQuality);

    void valueAdded(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value);
    void valueChanged(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value);
    void valueRemoved(const QUuid &networkUuid, quint8 nodeId, quint64 valueId);

protected:
    void startReply(ZWaveReply *reply, int timeout = 5000);
    void finishReply(ZWaveReply *reply, ZWave::ZWaveError error = ZWave::ZWaveErrorNoError);
};

Q_DECLARE_INTERFACE(ZWaveBackend, "io.nymea.ZWaveBackend")

#endif // ZWAVEBACKEND_H
