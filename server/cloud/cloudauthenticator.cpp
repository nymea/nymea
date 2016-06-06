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

#include "cloudauthenticator.h"

#include <QJsonParseError>
#include <QJsonDocument>

namespace guhserver {

CloudAuthenticator::CloudAuthenticator(QString clientId, QString clientSecret, QObject *parent) :
    QObject(parent),
    m_clientId(clientId),
    m_clientSecret(clientSecret),
    m_authenticated(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &CloudAuthenticator::replyFinished);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);

    connect(m_timer, &QTimer::timeout, this, &CloudAuthenticator::refreshTimeout);
}

QUrl CloudAuthenticator::url() const
{
    return m_url;
}

void CloudAuthenticator::setUrl(const QUrl &url)
{
    m_url = url;
}

QUrlQuery CloudAuthenticator::query() const
{
    return m_query;
}

void CloudAuthenticator::setQuery(const QUrlQuery &query)
{
    m_query = query;
}

QString CloudAuthenticator::username() const
{
    return m_username;
}

void CloudAuthenticator::setUsername(const QString &username)
{
    m_username = username;
}

QString CloudAuthenticator::password() const
{
    return m_password;
}

void CloudAuthenticator::setPassword(const QString &password)
{
    m_password = password;
}

QString CloudAuthenticator::clientId() const
{
    return m_clientId;
}

void CloudAuthenticator::setClientId(const QString &clientId)
{
    m_clientId = clientId;
}

QString CloudAuthenticator::clientSecret() const
{
    return m_clientSecret;
}

void CloudAuthenticator::setClientSecret(const QString clientSecret)
{
    m_clientSecret = clientSecret;
}

QString CloudAuthenticator::scope() const
{
    return m_scope;
}

void CloudAuthenticator::setScope(const QString &scope)
{
    m_scope = scope;
}

QString CloudAuthenticator::token() const
{
    return m_token;
}

bool CloudAuthenticator::authenticated() const
{
    return m_authenticated;
}

void CloudAuthenticator::startAuthentication()
{
    qCDebug(dcCloud()) << "Start authentication" << m_username;

    QUrlQuery query;
    query.addQueryItem("grant_type", "password");
    query.addQueryItem("username", m_username);
    query.addQueryItem("password", m_password);
    setQuery(query);

    QNetworkRequest request(m_url);
    QByteArray data = QString(m_clientId + ":" + m_clientSecret).toUtf8().toBase64();
    QString header = "Basic " + data;
    request.setRawHeader("Authorization", header.toLocal8Bit());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    m_tokenRequests.append(m_networkManager->post(request, m_query.toString().toUtf8()));
}

void CloudAuthenticator::setAuthenticated(const bool &authenticated)
{
    if (!authenticated) {
        m_timer->stop();
        qCWarning(dcCloud()) << "Authentication failed" << m_username;
    }
    m_authenticated = authenticated;
    emit authenticationChanged();
}

void CloudAuthenticator::setToken(const QString &token)
{
    m_token = token;
    emit tokenChanged();
}

void CloudAuthenticator::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // token request
    if (m_tokenRequests.contains(reply)) {

        QByteArray data = reply->readAll();
        m_tokenRequests.removeAll(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcCloud()) << "Request token reply HTTP error:" << status << reply->errorString();
            qCWarning(dcCloud()) << data;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check JSON
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcCloud()) << "Request token reply JSON error:" << error.errorString();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        if (!jsonDoc.toVariant().toMap().contains("access_token")) {
            qCWarning(dcCloud()) << "Could not get access token" << jsonDoc.toJson();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        setToken(jsonDoc.toVariant().toMap().value("access_token").toString());
        setAuthenticated(true);

        if (jsonDoc.toVariant().toMap().contains("expires_in") && jsonDoc.toVariant().toMap().contains("refresh_token")) {
            int expireTime = jsonDoc.toVariant().toMap().value("expires_in").toInt();
            m_refreshToken = jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcCloud()) << "Token will be refreshed in" << expireTime << "[s]";
            m_timer->start((expireTime - 20) * 1000);
        }

    } else if (m_refreshTokenRequests.contains(reply)) {

        QByteArray data = reply->readAll();
        m_refreshTokenRequests.removeAll(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcCloud()) << "Refresh token reply HTTP error:" << status << reply->errorString();
            qCWarning(dcCloud()) << data;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check JSON
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcCloud()) << "Refresh token reply JSON error:" << error.errorString();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        if (!jsonDoc.toVariant().toMap().contains("access_token")) {
            qCWarning(dcCloud()) << "Could not get access token after refresh" << jsonDoc.toJson();
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        setToken(jsonDoc.toVariant().toMap().value("access_token").toString());
        qCDebug(dcCloud()) << "Token refreshed successfully";

        if (jsonDoc.toVariant().toMap().contains("expires_in") && jsonDoc.toVariant().toMap().contains("refresh_token")) {
            int expireTime = jsonDoc.toVariant().toMap().value("expires_in").toInt();
            m_refreshToken = jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcCloud()) << "Token will be refreshed in" << expireTime << "[s]";
            m_timer->start((expireTime - 20) * 1000);
        }

        if (!authenticated())
            setAuthenticated(true);
    }

    reply->deleteLater();
}

void CloudAuthenticator::refreshTimeout()
{
    qCDebug(dcCloud()) << "Refresh authentication token for" << m_username;

    QUrlQuery query;
    query.addQueryItem("grant_type", "refresh_token");
    query.addQueryItem("refresh_token", m_refreshToken);
    query.addQueryItem("client_id", m_clientId);
    query.addQueryItem("client_secret", m_clientSecret);

    QNetworkRequest request(m_url);
    QByteArray data = QString(m_clientId + ":" + m_clientSecret).toUtf8().toBase64();
    QString header = "Basic " + data;
    request.setRawHeader("Authorization", header.toLocal8Bit());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    m_refreshTokenRequests.append(m_networkManager->post(request, query.toString().toUtf8()));
}

}
