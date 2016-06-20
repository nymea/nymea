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
    returns.insert("cloudError", JsonTypes::cloudErrorRef());
    setReturns("Authenticate", returns);

    params.clear(); returns.clear();
    setDescription("GetConnectionStatus", "Get the current status of the cloud connection.");
    setParams("GetConnectionStatus", params);
    returns.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("connected", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("active", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("authenticated", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("GetConnectionStatus", returns);

    params.clear(); returns.clear();
    setDescription("Enable", "Enable the cloud connection.");
    setParams("Enable", params);
    returns.insert("cloudError", JsonTypes::cloudErrorRef());
    setReturns("Enable", returns);

    params.clear(); returns.clear();
    setDescription("Disable", "Disable the cloud connection.");
    setParams("Disable", params);
    returns.insert("cloudError", JsonTypes::cloudErrorRef());
    setReturns("Disable", returns);

    // Notification
    params.clear(); returns.clear();
    setDescription("ConnectionStatusChanged", "Emitted whenever the status of the cloud connection changed. The cloud connection is active if a cloud client is talking with the server.");
    params.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("connected", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("active", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("authenticated", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("ConnectionStatusChanged", params);

    connect(GuhCore::instance()->cloudManager(), SIGNAL(enabledChanged()), this, SLOT(onConnectionStatusChanged()));
    connect(GuhCore::instance()->cloudManager(), SIGNAL(connectedChanged()), this, SLOT(onConnectionStatusChanged()));
    connect(GuhCore::instance()->cloudManager(), SIGNAL(activeChanged()), this, SLOT(onConnectionStatusChanged()));
    connect(GuhCore::instance()->cloudManager(), SIGNAL(authenticatedChanged()), this, SLOT(onConnectionStatusChanged()));

    connect(GuhCore::instance()->cloudManager(), SIGNAL(authenticationFinished(Cloud::CloudError)), this, SLOT(onAuthenticationRequestFinished(Cloud::CloudError)));
}

QString CloudHandler::name() const
{
    return "Cloud";
}

JsonReply *CloudHandler::Authenticate(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();
    qCDebug(dcJsonRpc()) << "Authenticate cloud connection for user" << username;

    if (!GuhCore::instance()->cloudManager()->enabled()) {
        qCWarning(dcCloud()) << "Could not start authentication: cloud connection is disabled";
        return createReply(statusToReply(Cloud::CloudErrorCloudConnectionDisabled));
    }

    JsonReply *reply = createAsyncReply("Authenticate");
    m_asyncAuthenticationReplies.append(reply);
    GuhCore::instance()->cloudManager()->connectToCloud(username, password);
    return reply;
}

JsonReply *CloudHandler::GetConnectionStatus(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("enabled", GuhCore::instance()->cloudManager()->enabled());
    returns.insert("connected", GuhCore::instance()->cloudManager()->connected());
    returns.insert("active", GuhCore::instance()->cloudManager()->active());
    returns.insert("authenticated", GuhCore::instance()->cloudManager()->authenticated());
    return createReply(returns);
}

JsonReply *CloudHandler::Enable(const QVariantMap &params) const
{
    Q_UNUSED(params)
    GuhCore::instance()->configuration()->setCloudEnabled(true);
    return createReply(statusToReply(Cloud::CloudErrorNoError));
}

JsonReply *CloudHandler::Disable(const QVariantMap &params) const
{
    Q_UNUSED(params)
    GuhCore::instance()->configuration()->setCloudEnabled(false);
    return createReply(statusToReply(Cloud::CloudErrorNoError));
}

void CloudHandler::onConnectionStatusChanged()
{
    QVariantMap params;
    params.insert("enabled", GuhCore::instance()->cloudManager()->enabled());
    params.insert("connected", GuhCore::instance()->cloudManager()->connected());
    params.insert("active", GuhCore::instance()->cloudManager()->active());
    params.insert("authenticated", GuhCore::instance()->cloudManager()->authenticated());

    emit ConnectionStatusChanged(params);
}

void CloudHandler::onAuthenticationRequestFinished(const Cloud::CloudError &error)
{
    if (m_asyncAuthenticationReplies.isEmpty())
        return;

    foreach (JsonReply *reply, m_asyncAuthenticationReplies) {
        m_asyncAuthenticationReplies.removeOne(reply);
        reply->setData(statusToReply(error));
        reply->finished();
    }
}

}
