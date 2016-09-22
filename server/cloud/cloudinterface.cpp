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

#include "cloudinterface.h"
#include "loggingcategories.h"
#include "guhsettings.h"
#include "guhcore.h"

namespace guhserver {

CloudInterface::CloudInterface(QObject *parent) :
    QObject(parent)
{
    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("guhd");
    m_guhUuid = settings.value("uuid", QVariant()).toUuid();
    if (m_guhUuid.isNull()) {
        m_guhUuid = QUuid::createUuid().toString();
        settings.setValue("uuid", m_guhUuid);
    }
    settings.endGroup();

    m_authenticationHandler = new CloudAuthenticationHandler(this);
    m_connectionHandler = new CloudConnectionHandler(this);

    m_handlers.insert(m_authenticationHandler->nameSpace(), m_authenticationHandler);
    m_handlers.insert(m_connectionHandler->nameSpace(), m_connectionHandler);
}

void CloudInterface::authenticateConnection(const QString &token)
{
    qCDebug(dcCloud()) << "Start cloud connection authentication" << token;

    QVariantMap params;
    // TODO: use server/station name
    params.insert("name", "guhIO");
    params.insert("id", m_guhUuid);
    params.insert("token", token);
    params.insert("type", "ConnectionTypeServer");

    CloudJsonReply *reply = createReply("Authentication", "Authenticate", params);
    GuhCore::instance()->cloudManager()->sendCloudData(reply->requestMap());
    m_replies.insert(reply->commandId(), reply);
}

void CloudInterface::getTunnels()
{
    CloudJsonReply *reply = createReply("Connection", "GetTunnels");
    GuhCore::instance()->cloudManager()->sendCloudData(reply->requestMap());
    m_replies.insert(reply->commandId(), reply);
}

void CloudInterface::sendApiData(const QUuid &tunnelId, const QVariantMap &data)
{
    //qCDebug(dcCloud()) << "Send API data" << tunnelId.toString() << data;

    QVariantMap params;
    params.insert("tunnelId", tunnelId.toString());
    params.insert("data", data);

    CloudJsonReply *reply = createReply("Connection", "SendData", params);
    GuhCore::instance()->cloudManager()->sendCloudData(reply->requestMap());
    m_replies.insert(reply->commandId(), reply);
}

CloudJsonReply *CloudInterface::createReply(QString nameSpace, QString method, QVariantMap params)
{
    m_id++;
    return new CloudJsonReply(m_id, nameSpace, method, params, this);
}

void CloudInterface::dataReceived(const QVariantMap &data)
{
    int commandId = data.value("id").toInt();
    QPointer<CloudJsonReply> reply = m_replies.take(commandId);

    QVariantMap params = data.value("params").toMap();

    // check if this is a reply to a request
    if (!reply.isNull() && !data.contains("notification")) {
        //qCDebug(dcCloud()) << "JsonRpc: got response for" << QString("%1.%2").arg(reply->nameSpace(), reply->method());
        CloudJsonHandler *handler = m_handlers.value(reply->nameSpace());

        if (!QMetaObject::invokeMethod(handler, QString("process" + reply->method()).toLatin1().data(), Q_ARG(QVariantMap, params)))
            qCWarning(dcCloud()) << "JsonRpc: method not implemented:" << reply->method();

        reply->deleteLater();
        return;
    }

    // check if this is a notification
    if (data.contains("notification")) {
        QStringList notification = data.value("notification").toString().split(".");
        QString nameSpace = notification.first();
        QString method = notification.last();
        CloudJsonHandler *handler = m_handlers.value(nameSpace);

        if (!handler) {
            qCWarning(dcCloud()) << "JsonRpc: handler not implemented:" << nameSpace;
            return;
        }

        if (!QMetaObject::invokeMethod(handler, QString("process" + method).toLatin1().data(), Q_ARG(QVariantMap, params)))
            qCWarning(dcCloud()) << "JsonRpc: Method not implemented";

    }
}

}
