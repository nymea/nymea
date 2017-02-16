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

#ifndef CLOUDAUTHENTICATOR_H
#define CLOUDAUTHENTICATOR_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "loggingcategories.h"

namespace guhserver {

class CloudAuthenticator : public QObject
{
    Q_OBJECT
public:
    explicit CloudAuthenticator(QString clientId, QString clientSecret, QObject *parent = 0);

    QUrl url() const;
    void setUrl(const QUrl &url);

    QUrlQuery query() const;
    void setQuery(const QUrlQuery &query);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    QString clientId() const;
    void setClientId(const QString &clientId);

    QString clientSecret() const;
    void setClientSecret(const QString clientSecret);

    // Read only
    QString token() const;
    bool authenticated() const;

    bool startAuthentication();
    void stopAuthentication();

private:
    QNetworkAccessManager *m_networkManager;
    QTimer *m_timer;
    QList<QNetworkReply *> m_tokenRequests;
    QList<QNetworkReply *> m_refreshTokenRequests;

    QUrl m_url;
    QUrlQuery m_query;
    QString m_username;
    QString m_password;
    QString m_clientId;
    QString m_clientSecret;

    QString m_token;
    QString m_refreshToken;

    bool m_authenticated;

    void setAuthenticated(const bool &authenticated);
    void setToken(const QString &token);

    void setRefreshToken(const QString &refreshToken);
    QString loadRefreshToken() const;
    QString loadUserName() const;

private slots:
    void replyFinished(QNetworkReply *reply);
    void refreshTimeout();
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

signals:
    void authenticationChanged();
    void tokenChanged();

};

}

#endif // CLOUDAUTHENTICATOR_H
