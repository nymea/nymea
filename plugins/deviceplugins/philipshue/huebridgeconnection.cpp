/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2013 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include "huebridgeconnection.h"

#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

HueBridgeConnection::HueBridgeConnection(QObject *parent) :
    QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

int HueBridgeConnection::createUser(const QHostAddress &address, const QString &username)
{
    QVariantMap createUserParams;
    createUserParams.insert("devicetype", "guh");
    createUserParams.insert("username", username);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(createUserParams);
    QByteArray data = jsonDoc.toJson();

    QNetworkRequest request(QUrl("http://" + address.toString() + "/api"));
    QNetworkReply *reply = m_nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &HueBridgeConnection::slotCreateUserFinished);

    m_createUserMap.insert(reply, m_requestCounter);
    return m_requestCounter++;
}

int HueBridgeConnection::get(const QHostAddress &address, const QString &username, const QString &path, QObject *caller, const QString &methodName)
{
    QString baseUrl = "http://" + address.toString() + "/api/" + username + "/";
    QUrl url(baseUrl + path);

    QNetworkRequest request;
    request.setUrl(url);
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &HueBridgeConnection::slotGetFinished);

    Caller c;
    c.obj = caller;
    c.method = methodName;
    c.id = m_requestCounter;

    m_requestMap.insert(reply, c);
    return m_requestCounter++;
}

int HueBridgeConnection::put(const QHostAddress &address, const QString &username, const QString &path, const QVariantMap &data, QObject *caller, const QString &methodName)
{
    QString baseUrl = "http://" + address.toString() + "/api/" + username + "/";
    QUrl url(baseUrl + path);
    QNetworkRequest request;
    request.setUrl(url);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(data);
    QByteArray jsonData = jsonDoc.toJson();
    qDebug() << "putting" << url << jsonData;

    QNetworkReply *reply = m_nam->put(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, &HueBridgeConnection::slotGetFinished);

    Caller c;
    c.obj = caller;
    c.method = methodName;
    c.id = m_requestCounter;
    m_requestMap.insert(reply, c);

    return m_requestCounter++;
}

void HueBridgeConnection::slotCreateUserFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    QByteArray data = reply->readAll();
    int id = m_createUserMap.take(reply);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        QVariantMap params;
        QVariantMap errorMap;
        errorMap.insert("description", "Failed to parse the bridge's response:" + error.errorString());
        params.insert("error", errorMap);
        emit createUserFinished(id, params);
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toList().first().toMap();
    emit createUserFinished(id, response);
}

void HueBridgeConnection::slotGetFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    QByteArray data = reply->readAll();
    Caller c = m_requestMap.take(reply);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        QVariantMap params;
        QVariantMap errorMap;
        errorMap.insert("description", "Failed to parse the bridge's response:" + error.errorString());
        params.insert("error", errorMap);
        emit createUserFinished(c.id, params);
        return;
    }

    QVariant response = jsonDoc.toVariant();
    emit getFinished(c.id, response.toMap());
    if (c.obj) {
        metaObject()->invokeMethod(c.obj.data(), c.method.toLatin1().data(), Q_ARG(int, c.id), Q_ARG(QVariant, response));
    }
}
