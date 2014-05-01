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

class SmtpClient : public QObject
{
    Q_OBJECT
public:

    enum AuthMethod{
        AuthPlain,
        AuthLogin
    };

    enum SendState{
        InitState,
        HandShakeState,
        AuthentificationState,
        UserState,
        PasswordState,
        MailState,
        RcptState,
        DataState,
        BodyState,
        QuitState,
        CloseState
    };

    enum ConnectionType{
        TcpConnection,      // no encryption
        SslConnection,      // SSL
        TlsConnection       // STARTTLS
    };

    explicit SmtpClient();
    void connectToHost();
    void login(const QString &user, const QString &password);
    void logout();
    bool sendMail(const QString &from, const QString &to, const QString &subject, const QString &body);

    void setHost(const QString &host);
    void setPort(const int &port);
    void setConnectionType(const ConnectionType &connectionType);
    void setAuthMethod(const AuthMethod &authMethod);
    void setUser(const QString &user);
    void setPassword(const QString &password);
    void setRecipiant(const QString &rcpt);


private:
    SendState m_state;
    QSslSocket *m_socket;
    QString m_host;
    int m_port;
    ConnectionType m_connectionType;
    AuthMethod m_authMethod;
    QString m_user;
    QString m_password;
    QString m_from;
    QString m_rcpt;
    QString m_subject;
    QString m_boy;
    QString m_message;

signals:

private slots:
    void socketError(QAbstractSocket::SocketError error);
    void connected();
    void readData();
    void disconnected();
    void send(const QString &data);

public slots:



};

#endif // SMTPCLIENT_H
