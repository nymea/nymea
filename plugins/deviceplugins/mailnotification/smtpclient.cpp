/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "smtpclient.h"
#include "extern-plugininfo.h"

SmtpClient::SmtpClient(QObject *parent):
    QObject(parent)
{
    m_socket = new QSslSocket(this);
    m_state = InitState;
    m_testLogin = false;

    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    // error because QSslSocket has also a method called error...also QAbstractSocket or QTcpSocket don't work...
    //connect(m_socket, &QSslSocket::error, this, &SmtpClient::socketError);
    connect(m_socket, &QSslSocket::connected, this, &SmtpClient::connected);
    connect(m_socket, &QSslSocket::readyRead, this, &SmtpClient::readData);
    connect(m_socket, &QSslSocket::disconnected, this, &SmtpClient::disconnected);

}

void SmtpClient::connectToHost()
{
    switch (m_encryptionType) {
    case EncryptionTypeNone:
        m_socket->connectToHost(m_host, m_port);
        break;
    case EncryptionTypeSSL:
        m_socket->connectToHostEncrypted(m_host, m_port);
        break;
    case EncryptionTypeTLS:
        m_socket->connectToHost(m_host,m_port);
        break;
    default:
        break;
    }
}

void SmtpClient::testLogin()
{
    m_socket->close();
    m_testLogin = true;
    connectToHost();
}

void SmtpClient::connected()
{
}

void SmtpClient::disconnected()
{
}

void SmtpClient::readData()
{
    QString response;
    QString responseLine;

    while(m_socket->canReadLine() && responseLine[3] != ' ') {
        responseLine = m_socket->readLine();
        response.append(responseLine);
    }
    responseLine.truncate( 3 );

    switch (m_state) {
    case InitState:
        if(responseLine == "220") {
            send("EHLO localhost");
            if(m_encryptionType == EncryptionTypeNone) {
                m_state = AuthentificationState;
                break;
            }
            if(m_encryptionType == EncryptionTypeSSL) {
                m_state = HandShakeState;
                break;
            }
            if(m_encryptionType == EncryptionTypeTLS) {
                m_state = StartTlsState;
                break;
            }
        }
        break;
    case HandShakeState:
        if(responseLine == "250") {
            if(!m_socket->isEncrypted() && m_encryptionType != EncryptionTypeNone) {
                m_socket->startClientEncryption();
            }
            send("EHLO localhost");
            m_state = AuthentificationState;
        }
        if(responseLine == "220") {
            if(!m_socket->isEncrypted() && m_encryptionType != EncryptionTypeNone) {
                m_socket->startClientEncryption();
            }
            send("EHLO localhost");
            m_state = AuthentificationState;
        }
        break;
    case StartTlsState:
        if(responseLine == "250") {
            send("STARTTLS");
            m_state = HandShakeState;
        }
        break;
    case AuthentificationState:
        if(responseLine == "250") {
            if(m_authMethod == AuthMethodLogin) {
                send("AUTH LOGIN");
                m_state = UserState;
                break;
            }
            if(m_authMethod == AuthMethodPlain) {
                send("AUTH PLAIN " + QByteArray().append((char) 0).append(m_user).append((char) 0).append(m_password).toBase64());
                // if we just want to test the Login, we are almost done here
                if(!m_testLogin) {
                    m_state = MailState;
                } else {
                    m_state = TestLoginFinishedState;
                }
                break;
            }
        }
        break;
    case UserState:
        if(responseLine == "334") {
            send(QByteArray().append(m_user).toBase64());
            m_state = PasswordState;
        }
        break;
    case PasswordState:
        if(responseLine == "334") {
            send(QByteArray().append(m_password).toBase64());
            // if we just want to test the Login, we are almost done here
            if(!m_testLogin) {
                m_state = MailState;
            } else {
                m_state = TestLoginFinishedState;
            }
            break;
        }
        break;
    case TestLoginFinishedState:
        if(responseLine == "235") {
            emit testLoginFinished(true);
        } else {
            emit testLoginFinished(false);
        }
        m_socket->close();
        m_testLogin = false;
        break;
    case MailState:
        if(responseLine == "235") {
            send("MAIL FROM:<" + m_sender + ">");
            m_state = RcptState;
        }
        break;
    case RcptState:
        if(responseLine == "250") {
            send("RCPT TO:<" + m_rcpt + ">");
            m_state = DataState;
        }
        break;
    case DataState:
        if(responseLine == "250") {
            send("DATA");
            m_state = BodyState;
        }
        break;
    case BodyState:
        if(responseLine == "354") {
            send(m_message + "\r\n.\r\n");
            m_state = QuitState;
        }
        break;
    case QuitState:
        if(responseLine == "250") {
            emit sendMailFinished(true, m_actionId);
            send("QUIT");
            m_state = CloseState;
        }
        break;
    case CloseState:
        if(responseLine == "221") {
            m_socket->close();
        }
        // some mail server does not recognize the QUIT command...so close the connection either way
        m_socket->close();
        break;
    default:
        // unexpecterd response code received...
        if(m_testLogin) {
            emit testLoginFinished(false);
            m_testLogin = false;
            m_socket->close();
            break;
        }
        emit sendMailFinished(false, m_actionId);
        m_socket->close();
        break;
    }
}

bool SmtpClient::sendMail(const QString &subject, const QString &body, const ActionId &actionId)
{
    m_actionId = actionId;

    // create mail content
    m_state = InitState;
    m_message.clear();

    m_message = "To: " + m_rcpt + "\n";
    m_message.append("From: " + m_sender + "\n");
    m_message.append("Subject: [guh notification] | " + subject + "\n");
    m_message.append(body);
    m_message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );
    m_message.replace( QString::fromLatin1( "\r\n.\r\n" ), QString::fromLatin1( "\r\n..\r\n" ) );
    m_message.append("\r\n.\r\n");
    m_socket->close();
    connectToHost();

    return true;
}

void SmtpClient::setHost(const QString &host)
{
    m_host = host;
}

void SmtpClient::setPort(const int &port)
{
    m_port = port;
}

void SmtpClient::setEncryptionType(const SmtpClient::EncryptionType &encryptionType)
{
    m_encryptionType = encryptionType;
}

void SmtpClient::setAuthMethod(const SmtpClient::AuthMethod &authMethod)
{
    m_authMethod = authMethod;
}

void SmtpClient::setUser(const QString &user)
{
    m_user = user;
}

void SmtpClient::setPassword(const QString &password)
{
    m_password = password;
}

void SmtpClient::setSender(const QString &sender)
{
    m_sender = sender;
}

void SmtpClient::setRecipient(const QString &rcpt)
{
    m_rcpt = rcpt;
}

void SmtpClient::socketError(QAbstractSocket::SocketError error)
{
    qCWarning(dcMailNotification) << "ERROR: mail socket -> " << error << m_socket->errorString();
}

void SmtpClient::send(const QString &data)
{
    m_socket->write(data.toUtf8() + "\r\n");
    m_socket->flush();
}


