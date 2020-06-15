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

#ifndef MODBUSCLIENTMANAGERIMPL_H
#define MODBUSCLIENTMANAGERIMPL_H

#include "libnymea.h"
#include "typeutils.h"
#include "network/modbusclientmanager.h"

#include <QObject>
#include <QModbusClient>
#include <QModbusReply>
#include <QModbusRequest>
#include <QModbusDataUnit>
#include <QDebug>
#include <QUrl>
#include <QTimer>

namespace nymeaserver {

class NetworkAccessManagerImpl : public ModbusClientManager
{
    Q_OBJECT

public:
    ModbusClientManagerImpl(QModbusClient *modbusClient, QObject *parent = nullptr);

    QModbusReply *sendReadRequest(const QModbusDataUnit &read, int serverAddress) override;
    QModbusReply *sendWriteRequest(const QModbusDataUnit &write, int serverAddress) override;
    QModbusReply *sendReadWriteRequest(const QModbusDataUnit &read, const QModbusDataUnit &write, int serverAddress) override;
    QModbusReply *sendRawRequest(const QModbusRequest &request, int serverAddress) override;
    bool enabled() const override;

protected:
    void setEnabled(bool enabled) override;

private:
    bool m_available = false;
    bool m_enabled = false;

    QModbusClient *m_modbusClient;
    QHash<QModbusReply*, QTimer*> m_timeoutTimers;

    void hookupTimeoutTimer(QNetworkReply* reply);

private slots:
    void networkReplyFinished();
    void networkTimeout();

};

}

#endif // MODBUSCLIENTMANAGER_H
