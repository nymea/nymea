/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "kodijsonhandler.h"
#include "extern-plugininfo.h"

#include <QJsonDocument>

KodiJsonHandler::KodiJsonHandler(KodiConnection *connection, QObject *parent) :
    QObject(parent),
    m_connection(connection),
    m_id(0)
{
    connect(m_connection, &KodiConnection::dataReady, this, &KodiJsonHandler::processResponse);
}

void KodiJsonHandler::sendData(const QString &method, const QVariantMap &params, const ActionId &actionId)
{
    QVariantMap package;
    package.insert("id", m_id);
    package.insert("method", method);
    package.insert("params", params);
    package.insert("jsonrpc", "2.0");

    m_replys.insert(m_id, KodiReply(method, params, actionId));

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(package);
    m_connection->sendData(jsonDoc.toJson());
    //qCDebug(dcKodi) << "sending data" << jsonDoc.toJson();
    m_id++;
}

void KodiJsonHandler::processNotification(const QString &method, const QVariantMap &params)
{
    //qCDebug(dcKodi) << "got notification" << method;

    if (method == "Application.OnVolumeChanged") {
        QVariantMap data = params.value("data").toMap();
        emit volumeChanged(data.value("volume").toInt(), data.value("muted").toBool());
    } else if (method == "Player.OnPlay") {
        emit onPlayerPlay();
    } else if (method == "Player.OnPause") {
        emit onPlayerPause();
    } else if (method == "Player.OnStop") {
        emit onPlayerStop();
    }
}

void KodiJsonHandler::processActionResponse(const KodiReply &reply, const QVariantMap &response)
{
    if (response.contains("error")) {
        //qCDebug(dcKodi) << QJsonDocument::fromVariant(response).toJson();
        qCWarning(dcKodi) << "got error response for action"  << reply.method() << ":" << response.value("error").toMap().value("message").toString();
        emit actionExecuted(reply.actionId(), false);
    } else {
        emit actionExecuted(reply.actionId(), true);
    }
}

void KodiJsonHandler::processRequestResponse(const KodiReply &reply, const QVariantMap &response)
{
    if (response.contains("error")) {
        //qCDebug(dcKodi) << QJsonDocument::fromVariant(response).toJson();
        qCWarning(dcKodi) << "got error response for request " << reply.method() << ":" << response.value("error").toMap().value("message").toString();
    }

    if (reply.method() == "Application.GetProperties") {
        //qCDebug(dcKodi) << "got update response" << reply.method();
        emit updateDataReceived(response.value("result").toMap());
    }

    if (reply.method() == "JSONRPC.Version") {
        qCDebug(dcKodi) << "got version response" << reply.method();
        emit versionDataReceived(response.value("result").toMap());
    }
}

void KodiJsonHandler::processResponse(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcKodi) << "failed to parse JSON data:" << data << ":" << error.errorString();
        return;
    }

    //qCDebug(dcKodi) << "data received:" << jsonDoc.toJson();

    QVariantMap message = jsonDoc.toVariant().toMap();

    // check jsonrpc value
    if (!message.contains("jsonrpc") || message.value("jsonrpc").toString() != "2.0") {
        qCWarning(dcKodi) << "jsonrpc 2.0 value missing in message" << data;
    }

    // check id (if there is no id, it's an notification from kodi)
    if (!message.contains("id")) {

        // check method
        if (!message.contains("method")) {
            qCWarning(dcKodi) << "method missing in message" << data;
        }

        processNotification(message.value("method").toString(), message.value("params").toMap());
        return;
    }

    int id = message.value("id").toInt();
    KodiReply reply = m_replys.take(id);

    // check if this message is a response to an action call
    if (reply.actionId() != ActionId()) {
        processActionResponse(reply, message);
        return;
    }

    processRequestResponse(reply, message);
}
