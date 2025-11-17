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

#include "oauth2.h"
#include "loggingcategories.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QJsonDocument>

OAuth2::OAuth2(QString clientId, QString clientSecret, QObject *parent) :
    QObject(parent),
    m_clientId(clientId),
    m_clientSecret(clientSecret),
    m_authenticated(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &OAuth2::replyFinished);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);

    connect(m_timer, &QTimer::timeout, this, &OAuth2::refreshTimeout);
}

QUrl OAuth2::url() const
{
    return m_url;
}

void OAuth2::setUrl(const QUrl &url)
{
    m_url = url;
}

QUrlQuery OAuth2::query() const
{
    return m_query;
}

void OAuth2::setQuery(const QUrlQuery &query)
{
    m_query = query;
}

QString OAuth2::username() const
{
    return m_username;
}

void OAuth2::setUsername(const QString &username)
{
    m_username = username;
}

QString OAuth2::password() const
{
    return m_password;
}

void OAuth2::setPassword(const QString &password)
{
    m_password = password;
}

QString OAuth2::clientId() const
{
    return m_clientId;
}

void OAuth2::setClientId(const QString &clientId)
{
    m_clientId = clientId;
}

QString OAuth2::clientSecret() const
{
    return m_clientSecret;
}

void OAuth2::setClientSecret(const QString clientSecret)
{
    m_clientSecret = clientSecret;
}

QString OAuth2::scope() const
{
    return m_scope;
}

void OAuth2::setScope(const QString &scope)
{
    m_scope = scope;
}

QString OAuth2::token() const
{
    return m_token;
}

bool OAuth2::authenticated() const
{
    return m_authenticated;
}

void OAuth2::startAuthentication()
{
    qCDebug(dcOAuth2) << "Start authentication" << m_username;

    QUrlQuery query;
    query.addQueryItem("grant_type", "password");
    query.addQueryItem("client_id", m_clientId);
    query.addQueryItem("client_secret", m_clientSecret);
    query.addQueryItem("username", m_username);
    query.addQueryItem("password", m_password);
    query.addQueryItem("scope", m_scope);
    setQuery(query);

    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    m_tokenRequests.append(m_networkManager->post(request, m_query.toString().toUtf8()));
}

void OAuth2::setAuthenticated(const bool &authenticated)
{
    if (authenticated) {
        qCDebug(dcOAuth2) << "Authenticated successfully" << m_username;
    } else {
        m_timer->stop();
        qCWarning(dcOAuth2) << "Authentication failed" << m_username;
    }
    m_authenticated = authenticated;
    emit authenticationChanged();
}

void OAuth2::setToken(const QString &token)
{
    m_token = token;
    emit tokenChanged();
}

void OAuth2::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // token request
    if (m_tokenRequests.contains(reply)) {

        QByteArray data = reply->readAll();
        m_tokenRequests.removeAll(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcOAuth2) << "Request token reply HTTP error:" << status << reply->errorString();
            qCWarning(dcOAuth2) << data;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check JSON
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcOAuth2) << "Request token reply JSON error:" << error.errorString();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        if (!jsonDoc.toVariant().toMap().contains("access_token")) {
            qCWarning(dcOAuth2) << "Could not get access token" << jsonDoc.toJson();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        setToken(jsonDoc.toVariant().toMap().value("access_token").toString());
        setAuthenticated(true);

        if (jsonDoc.toVariant().toMap().contains("expires_in") && jsonDoc.toVariant().toMap().contains("refresh_token")) {
            int expireTime = jsonDoc.toVariant().toMap().value("expires_in").toInt();
            m_refreshToken = jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcOAuth2) << "Token will be refreshed in" << expireTime << "[s]";
            m_timer->start((expireTime - 20) * 1000);
        }

    } else if (m_refreshTokenRequests.contains(reply)) {

        QByteArray data = reply->readAll();
        m_refreshTokenRequests.removeAll(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcOAuth2) << "Refresh token reply HTTP error:" << status << reply->errorString();
            qCWarning(dcOAuth2) << data;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check JSON
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcOAuth2) << "Refresh token reply JSON error:" << error.errorString();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        if (!jsonDoc.toVariant().toMap().contains("access_token")) {
            qCWarning(dcOAuth2) << "Could not get access token after refresh" << jsonDoc.toJson();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        setToken(jsonDoc.toVariant().toMap().value("access_token").toString());
        qCDebug(dcOAuth2) << "Token refreshed successfully";

        if (jsonDoc.toVariant().toMap().contains("expires_in") && jsonDoc.toVariant().toMap().contains("refresh_token")) {
            int expireTime = jsonDoc.toVariant().toMap().value("expires_in").toInt();
            m_refreshToken = jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcOAuth2) << "Token will be refreshed in" << expireTime << "[s]";
            m_timer->start((expireTime - 20) * 1000);
        }

        if (!authenticated())
            setAuthenticated(true);
    }

    reply->deleteLater();
}

void OAuth2::refreshTimeout()
{
    qCDebug(dcOAuth2) << "Refresh authentication token for" << m_username;

    QUrlQuery query;
    query.addQueryItem("grant_type", "refresh_token");
    query.addQueryItem("refresh_token", m_refreshToken);
    query.addQueryItem("client_id", m_clientId);
    query.addQueryItem("client_secret", m_clientSecret);

    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    m_refreshTokenRequests.append(m_networkManager->post(request, query.toString().toUtf8()));
}
