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
#include "guhsettings.h"

#include <QJsonParseError>
#include <QJsonDocument>

namespace guhserver {

CloudAuthenticator::CloudAuthenticator(QString clientId, QString clientSecret, QObject *parent) :
    QObject(parent),
    m_clientId(clientId),
    m_clientSecret(clientSecret),
    m_authenticated(false),
    m_error(Cloud::CloudErrorNoError)
{
    m_refreshToken = loadRefreshToken();
    m_username = loadUserName();

    m_networkManager = new QNetworkAccessManager(this);
    m_networkManager->

    connect(m_networkManager, &QNetworkAccessManager::finished, this, &CloudAuthenticator::replyFinished);
    connect(m_networkManager, &QNetworkAccessManager::sslErrors, this, &CloudAuthenticator::onSslErrors);

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

QString CloudAuthenticator::token() const
{
    return m_token;
}

bool CloudAuthenticator::authenticated() const
{
    return m_authenticated;
}

bool CloudAuthenticator::startAuthentication()
{
    qCDebug(dcCloud()) << "Authenticator: Start authenticating" << m_username;

    // Check if we have username and password
    if(!m_username.isEmpty() && !m_password.isEmpty()) {
        m_refreshToken.clear();
        stopAuthentication();

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

        QNetworkReply *reply = m_networkManager->post(request, m_query.toString().toUtf8());
        m_tokenRequests.append(reply);
        return true;

    } else if (!m_refreshToken.isEmpty()) {
        // Use the refreshtoken if there is one
        refreshTimeout();
        return true;
    }

    qCWarning(dcCloud()) << "Authenticator: Cannot start authentication. There is no refresh token, username or password around.";
    stopAuthentication();
    m_error = Cloud::CloudErrorLoginCredentialsMissing;
    return false;
}

void CloudAuthenticator::stopAuthentication()
{
    m_timer->stop();
    m_authenticated = false;
    emit authenticationChanged();
}

Cloud::CloudError CloudAuthenticator::error() const
{
    return m_error;
}

void CloudAuthenticator::setAuthenticated(const bool &authenticated)
{
    if (!authenticated)
        m_timer->stop();

    m_authenticated = authenticated;
    emit authenticationChanged();
}

void CloudAuthenticator::setToken(const QString &token)
{
    m_token = token;
    emit tokenChanged();
}

void CloudAuthenticator::setRefreshToken(const QString &refreshToken)
{
    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("Cloud");
    settings.setValue("refreshToken", refreshToken);
    settings.endGroup();
    m_refreshToken = refreshToken;
}

QString CloudAuthenticator::loadRefreshToken() const
{
    QString refreshToken;
    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("Cloud");
    refreshToken = settings.value("refreshToken", QString()).toString();
    settings.endGroup();
    return refreshToken;
}

QString CloudAuthenticator::loadUserName() const
{
    QString userName;
    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("Cloud");
    userName = settings.value("userName", QString()).toString();
    settings.endGroup();
    return userName;
}

void CloudAuthenticator::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // token request
    if (m_tokenRequests.contains(reply)) {

        QByteArray data = reply->readAll();
        m_tokenRequests.removeAll(reply);

        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcCloud()) << "Authenticator: Request token reply error:" << status << reply->errorString();            
            m_error = Cloud::CloudErrorIdentityServerNotReachable;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcCloud()) << "Authenticator: Request token reply HTTP error:" << status << reply->errorString();
            qCWarning(dcCloud()) << data;
            m_error = Cloud::CloudErrorAuthenticationFailed;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check JSON
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcCloud()) << "Authenticator: Request token reply JSON error:" << error.errorString();
            m_error = Cloud::CloudErrorAuthenticationFailed;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        if (!jsonDoc.toVariant().toMap().contains("access_token")) {
            qCWarning(dcCloud()) << "Authenticator: Could not get access token" << jsonDoc.toJson();
            m_error = Cloud::CloudErrorAuthenticationFailed;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        m_error = Cloud::CloudErrorNoError;
        setToken(jsonDoc.toVariant().toMap().value("access_token").toString());
        setAuthenticated(true);

        // Save the username
        GuhSettings settings(GuhSettings::SettingsRoleDevices);
        settings.beginGroup("Cloud");
        settings.setValue("userName", m_username);
        settings.endGroup();

        if (jsonDoc.toVariant().toMap().contains("expires_in") && jsonDoc.toVariant().toMap().contains("refresh_token")) {
            int expireTime = jsonDoc.toVariant().toMap().value("expires_in").toInt();
            setRefreshToken(jsonDoc.toVariant().toMap().value("refresh_token").toString());
            qCDebug(dcCloud()) << "Authenticator: Token will be refreshed in" << expireTime << "[s]";
            m_timer->start((expireTime - 20) * 1000);
        }

    } else if (m_refreshTokenRequests.contains(reply)) {

        QByteArray data = reply->readAll();
        m_refreshTokenRequests.removeAll(reply);

        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(dcCloud()) << "Authenticator: Request token reply error:" << status << reply->errorString();
            m_error = Cloud::CloudErrorIdentityServerNotReachable;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcCloud()) << "Authenticator: Refresh token reply HTTP error:" << status << reply->errorString();
            m_error = Cloud::CloudErrorAuthenticationFailed;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        // check JSON
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcCloud()) << "Authenticator: Refresh token reply JSON error:" << error.errorString();
            m_error = Cloud::CloudErrorAuthenticationFailed;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        if (!jsonDoc.toVariant().toMap().contains("access_token")) {
            qCWarning(dcCloud()) << "Authenticator: Could not get access token after refresh" << jsonDoc.toJson();
            m_error = Cloud::CloudErrorAuthenticationFailed;
            setAuthenticated(false);
            reply->deleteLater();
            return;
        }

        m_error = Cloud::CloudErrorNoError;
        setToken(jsonDoc.toVariant().toMap().value("access_token").toString());
        qCDebug(dcCloud()) << "Authenticator: Token refreshed successfully";

        if (jsonDoc.toVariant().toMap().contains("expires_in") && jsonDoc.toVariant().toMap().contains("refresh_token")) {
            int expireTime = jsonDoc.toVariant().toMap().value("expires_in").toInt();
            setRefreshToken(jsonDoc.toVariant().toMap().value("refresh_token").toString());
            qCDebug(dcCloud()) << "Authenticator: Token will be refreshed in" << expireTime << "[s]";
            m_timer->start((expireTime - 20) * 1000);
        }

        if (!authenticated())
            setAuthenticated(true);

    }

    reply->deleteLater();
}

void CloudAuthenticator::refreshTimeout()
{
    qCDebug(dcCloud()) << "Authenticator: Refresh authentication token for" << m_username;

    QUrlQuery query;
    query.addQueryItem("grant_type", "refresh_token");
    query.addQueryItem("refresh_token", m_refreshToken);

    QNetworkRequest request(m_url);
    QByteArray data = QString(m_clientId + ":" + m_clientSecret).toUtf8().toBase64();
    QString header = "Basic " + data;
    request.setRawHeader("Authorization", header.toLocal8Bit());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_networkManager->post(request, query.toString().toUtf8());
    m_refreshTokenRequests.append(reply);
}

void CloudAuthenticator::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    reply->ignoreSslErrors();
    //reply->ignoreSslErrors(QList<QSslError>() << QSslError(QSslError::SelfSignedCertificate) << QSslError(QSslError::CertificateUntrusted) << QSslError(QSslError::HostNameMismatch));

    if (m_refreshTokenRequests.contains(reply)) {
        qCWarning(dcCloud()) << "SSL errors occured for token refresh reply:";
    } else if (m_tokenRequests.contains(reply)) {
        qCWarning(dcCloud()) << "SSL errors occured for token reply:";
    } else {
        qCWarning(dcCloud()) << "SSL errors occured for unknown reply:";
    }

    foreach (const QSslError &error, errors) {
        qCWarning(dcCloud()) << "    " << (int)error.error() << error.errorString();
    }
}

}
