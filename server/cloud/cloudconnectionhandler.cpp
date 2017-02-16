/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "cloudconnectionhandler.h"
#include "loggingcategories.h"
#include "guhcore.h"

namespace guhserver {

CloudConnectionHandler::CloudConnectionHandler(QObject *parent) :
    CloudJsonHandler(parent)
{

}

QString CloudConnectionHandler::nameSpace() const
{
    return "Connection";
}

void CloudConnectionHandler::processGetConnections(const QVariantMap &params)
{
    Q_UNUSED(params)
}

void CloudConnectionHandler::processGetTunnels(const QVariantMap &params)
{
    foreach (const QVariant &tunnelVariant, params.value("tunnel").toList()) {
        QVariantMap tunnelMap = tunnelVariant.toMap();

        QUuid tunnelId = tunnelMap.value("id").toUuid();
        QUuid serverId = tunnelMap.value("serverConnection").toMap().value("id").toUuid();
        QUuid clientId = tunnelMap.value("clientConnection").toMap().value("id").toUuid();

        GuhCore::instance()->cloudManager()->onTunnelAdded(tunnelId, serverId, clientId);
    }
}

void CloudConnectionHandler::processSendData(const QVariantMap &params)
{
    Q_UNUSED(params)
}

void CloudConnectionHandler::processConnectionAdded(const QVariantMap &params)
{
    QString name = params.value("connection").toMap().value("name").toString();
    QUuid connectionId = params.value("connection").toMap().value("id").toUuid();
    qCDebug(dcCloud()) << "Authorized cloud connection added" << name << connectionId.toString();
}

void CloudConnectionHandler::processConnectionRemoved(const QVariantMap &params)
{
    qCDebug(dcCloud()) << "A cloud user connection has been removed" << params.value("connectionId").toString();
}

void CloudConnectionHandler::processTunnelAdded(const QVariantMap &params)
{
    QVariantMap tunnelMap = params.value("tunnel").toMap();

    QUuid tunnelId = tunnelMap.value("id").toUuid();
    QUuid serverId = tunnelMap.value("serverConnection").toMap().value("id").toUuid();
    QUuid clientId = tunnelMap.value("clientConnection").toMap().value("id").toUuid();

    GuhCore::instance()->cloudManager()->onTunnelAdded(tunnelId, serverId, clientId);
}

void CloudConnectionHandler::processTunnelRemoved(const QVariantMap &params)
{
    QUuid tunnelId = params.value("tunnelId").toUuid();
    GuhCore::instance()->cloudManager()->onTunnelRemoved(tunnelId);
}

void CloudConnectionHandler::processDataReceived(const QVariantMap &params)
{
    QUuid tunnelId = params.value("tunnelId").toUuid();
    GuhCore::instance()->cloudManager()->onCloudDataReceived(tunnelId, params.value("data").toMap());
}

}
