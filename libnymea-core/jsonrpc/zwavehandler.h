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

#ifndef ZWAVEHANDLER_H
#define ZWAVEHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "zwave/zwavemanager.h"

#include <QObject>

namespace nymeaserver
{

class ZWaveHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ZWaveHandler(ZWaveManager *zwaveManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *IsZWaveAvailable(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetSerialPorts(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetNetworks(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *FactoryResetNetwork(const QVariantMap &params);

    Q_INVOKABLE JsonReply *GetNodes(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddNode(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveNode(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveFailedNode(const QVariantMap &params);
    Q_INVOKABLE JsonReply *CancelPendingOperation(const QVariantMap &params);

signals:
    void NetworkAdded(const QVariantMap &params);
    void NetworkChanged(const QVariantMap &params);
    void NetworkRemoved(const QVariantMap &params);

    void NodeAdded(const QVariantMap &params);
    void NodeChanged(const QVariantMap &params);
    void NodeRemoved(const QVariantMap &params);

private slots:
    void onNetworkAdded(ZWaveNetwork *network);
    void onNetworkChanged(ZWaveNetwork *network);
    void onNetworkRemoved(const QUuid &networkUuid);

    void onNodeAdded(ZWaveNode *node);
    void onNodeChanged(ZWaveNode *node);
    void onNodeRemoved(ZWaveNode *node);

private:
    QVariantMap packNetwork(ZWaveNetwork *network);
    QVariantMap packNode(ZWaveNode *node);

private:
    ZWaveManager *m_zwaveManager = nullptr;
};

}

#endif // ZWAVEHANDLER_H
