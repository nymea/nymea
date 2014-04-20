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

    enum ConnectionType{
        TcpConnection,      // no encryption
        SslConnection,      // SSL
        TlsConnection       // STARTTLS
    };

    explicit SmtpClient(const QString &host = "smtp.gmail.com", int port = 465, ConnectionType connectionType = SslConnection);
    bool connectToHost();
    bool login(const QString &user, const QString &password, AuthMethod method = AuthLogin);
    bool logout();
    bool sendMail(const QString &from, const QString &to, const QString &subject, const QString &body);

private:
    QSslSocket *m_socket;
    QString m_host;
    int m_port;

    ConnectionType m_connectionType;
    AuthMethod m_authMethod;

    int m_responseCode;
    int m_connectionTimeout;
    int m_responseTimeout;
    int m_sendMessageTimeout;
    int waitForResponse();

signals:

private slots:
    void socketError(QAbstractSocket::SocketError error);
    void connected();
    void disconnected();
    void send(const QString &data);

public slots:



};

#endif // SMTPCLIENT_H
