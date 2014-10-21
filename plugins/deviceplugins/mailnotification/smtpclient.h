/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QSslSocket>

#include "plugin/deviceplugin.h"

class SmtpClient : public QObject
{
    Q_OBJECT
public:

    enum AuthMethod{
        AuthMethodPlain,
        AuthMethodLogin
    };

    enum SendState{
        InitState,
        HandShakeState,
        AuthentificationState,
        StartTlsState,
        UserState,
        PasswordState,
        TestLoginFinishedState,
        MailState,
        RcptState,
        DataState,
        BodyState,
        QuitState,
        CloseState
    };

    enum EncryptionType{
        EncryptionNone,  // no encryption
        EncryptionSSL,   // SSL
        EncryptionTLS    // STARTTLS
    };

    explicit SmtpClient(QObject *parent = 0);
    explicit SmtpClient(QString host = QString(), int port = 465, QString user = QString(), QString password = QString(), AuthMethod authMethod = AuthPlain, EncryptionType encryptionType = EncryptionNone, QObject *parent = 0);

    void connectToHost();
    void login(const QString &user, const QString &password);
    void logout();
    bool sendMail(const QString &subject, const QString &body, const ActionId &actionId);

    void setHost(const QString &host);
    void setPort(const int &port);
    void setEncryptionType(const EncryptionType &encryptionType);
    void setAuthMethod(const AuthMethod &authMethod);
    void setUser(const QString &user);
    void setPassword(const QString &password);
    void setSender(const QString &sender);
    void setRecipiant(const QString &rcpt);


private:
    QSslSocket *m_socket;
    SendState m_state;
    QString m_host;
    int m_port;
    QString m_user;
    QString m_password;
    QString m_sender;
    AuthMethod m_authMethod;
    EncryptionType m_encryptionType;
    QString m_rcpt;
    QString m_subject;
    QString m_boy;
    QString m_message;
    ActionId m_actionId;

signals:
    void sendMailFinished(const bool &success, const ActionId &actionId);

private slots:
    void socketError(QAbstractSocket::SocketError error);
    void connected();
    void disconnected();
    void readData();
    void send(const QString &data);
};

#endif // SMTPCLIENT_H
