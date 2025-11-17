// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ZWAVEMANAGER_H
#define ZWAVEMANAGER_H

#include <QObject>
#include <QTimer>

#include "hardware/zwave/zwave.h"
#include "hardware/zwave/zwavereply.h"
#include "zwaveadapter.h"
#include "zwavenetwork.h"
#include "zwavedevicedatabase.h"
#include "hardware/zwave/zwavenode.h"

#include "hardware/serialport/serialportmonitor.h"

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcZWave)

class ZWaveBackend;

namespace nymeaserver
{
class ZWaveNodeImplementation;

class ZWaveManager : public QObject
{
    Q_OBJECT
public:    


    explicit ZWaveManager(SerialPortMonitor *serialPortMonitor, QObject *parent = nullptr);
    ~ZWaveManager();

    bool available() const;
    bool enabled() const;
    void setEnabled(bool enabled);

    SerialPorts serialPorts() const;
    ZWaveNetworks networks() const;
    ZWaveNetwork* network(const QUuid &networkUuid) const;

    QPair<ZWave::ZWaveError, QUuid> createNetwork(const QString &serialPort);
    ZWave::ZWaveError removeNetwork(const QUuid &networkUuid);
    ZWave::ZWaveError factoryResetNetwork(const QUuid &networkUuid);

    ZWaveReply* addNode(const QUuid &networkUuid);
    ZWaveReply *removeNode(const QUuid &networkUuid);
    ZWaveReply *removeFailedNode(const QUuid &networkUuid, quint8 nodeId);
    ZWaveReply *cancelPendingOperation(const QUuid &networkUuid);

    void setValue(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value);

signals:
    void networkAdded(ZWaveNetwork *network);
    void networkChanged(ZWaveNetwork *network);
    void networkStateChanged(ZWaveNetwork *network);
    void networkRemoved(const QUuid &networkUuid);

    void nodeAdded(ZWaveNode *node);
    void nodeChanged(ZWaveNode *node);
    void nodeInitialized(ZWaveNode *node);
    void nodeRemoved(ZWaveNode *node);

private:
    bool loadBackend();
    void loadZWaveNetworks();
    void setupNode(ZWaveNodeImplementation *node);

    bool loadNetwork(ZWaveNetwork *network);
    void storeNetwork(ZWaveNetwork *network);

private slots:
    void onNetworkStarted(const QUuid &networkUuid);
    void onNetworkFailed(const QUuid &networkUuid);
    void onWaitingForNodeAdditionChanged(const QUuid &networkUuid, bool waitingForNodeAddition);
    void onWaitingForNodeRemovalChanged(const QUuid &networkUuid, bool waitingForNodeRemoval);

    void onNodeAdded(const QUuid &networkUuid, quint8 nodeId);
    void onNodeInitialized(const QUuid &networkUuid, quint8 nodeId);
    void onNodeRemoved(const QUuid &networkUuid, quint8 nodeId);
    void onNodeDataChanged(const QUuid &networkUuid, quint8 nodeId);
    void onNodeReachableStatus(const QUuid &networkUuid, quint8 nodeId, bool reachable);
    void onNodeFailedStatus(const QUuid &networkUuid, quint8 nodeId, bool failed);
    void onNodeSleepStatus(const QUuid &networkUuid, quint8 nodeId, bool sleeping);
    void onNodeLinkQualityStatus(const QUuid &networkUuid, quint8 nodeId, quint8 linkQuality);

    void onValueAdded(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value);
    void onValueChanged(const QUuid &networkUuid, quint8 nodeId, const ZWaveValue &value);
    void onValueRemoved(const QUuid &networkUuid, quint8 nodeId, quint64 valueId);


private:
    SerialPortMonitor *m_serialPortMonitor = nullptr;

    ZWaveBackend *m_backend = nullptr;

    QHash<QUuid, ZWaveNetwork*> m_networks;
    ZWaveAdapters m_adapters;

    QHash<QUuid, ZWaveDeviceDatabase*> m_dbs;

    QTimer m_statsTimer;
};

}

#endif // ZWAVEMANAGER_H
