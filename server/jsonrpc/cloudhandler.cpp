/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#include "cloudhandler.h"

namespace guhserver {

CloudHandler::CloudHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap returns;
    QVariantMap params;

    setDescription("Authenticate", "Connect and authenticate the cloud connection with the given username and password.");
    params.insert("username", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("password", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("Authenticate", params);
    returns.insert("cloudConnectionError", JsonTypes::cloudConnectionErrorRef());
    setReturns("Authenticate", returns);

    params.clear(); returns.clear();
    setDescription("GetConnectionStatus", "Get the current status of the cloud connection.");
    setParams("GetConnectionStatus", params);
    returns.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("connected", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("active", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("authenticated", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("GetConnectionStatus", returns);

    // Notification
    params.clear(); returns.clear();
    setDescription("ConnectionStatusChanged", "Emitted whenever the status of the cloud connection changed.");
    params.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("connected", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("active", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("authenticated", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("ConnectionStatusChanged", params);

    connect(GuhCore::instance()->cloudConnection(), SIGNAL(enabledChanged()), this, SLOT(onConnectionStatusChanged()));
    connect(GuhCore::instance()->cloudConnection(), SIGNAL(connectedChanged()), this, SLOT(onConnectionStatusChanged()));
    connect(GuhCore::instance()->cloudConnection(), SIGNAL(activeChanged()), this, SLOT(onConnectionStatusChanged()));
    connect(GuhCore::instance()->cloudConnection(), SIGNAL(authenticatedChanged()), this, SLOT(onConnectionStatusChanged()));
}

QString CloudHandler::name() const
{
    return "Cloud";
}

JsonReply *CloudHandler::Authenticate(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QString username = params.value("username").toString();
    QString password = params.value("password").toString();
    qCDebug(dcJsonRpc()) << "Authenticate cloud connection for user" << username;

    GuhCore::instance()->cloudConnection()->connectToCloud(username, password);

    QVariantMap returns;
    returns.insert("cloudConnectionError", JsonTypes::cloudConnectionErrorToString(CloudConnection::CloudConnectionErrorNoError));
    return createReply(returns);
}

JsonReply *CloudHandler::GetConnectionStatus(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("enabled", GuhCore::instance()->cloudConnection()->enabled());
    returns.insert("connected", GuhCore::instance()->cloudConnection()->connected());
    returns.insert("active", GuhCore::instance()->cloudConnection()->active());
    returns.insert("authenticated", GuhCore::instance()->cloudConnection()->authenticated());
    return createReply(returns);
}

void CloudHandler::onConnectionStatusChanged()
{
    QVariantMap params;
    params.insert("enabled", GuhCore::instance()->cloudConnection()->enabled());
    params.insert("connected", GuhCore::instance()->cloudConnection()->connected());
    params.insert("active", GuhCore::instance()->cloudConnection()->active());
    params.insert("authenticated", GuhCore::instance()->cloudConnection()->authenticated());

    emit ConnectionStatusChanged(params);
}

}
