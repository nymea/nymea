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

#include "smtpclient.h"

SmtpClient::SmtpClient(QObject *parent):
    QObject(parent)
{
    m_state = InitState;
    m_socket = new QSslSocket(this);

    connect(m_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(m_socket,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(m_socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

}

SmtpClient::SmtpClient(QString host, int port, QString user, QString password, SmtpClient::AuthMethod authMethod, SmtpClient::EncryptionType encryptionType, QObject *parent):
    m_host(host),
    m_port(port),
    m_user(user),
    m_password(password),
    m_authMethod(authMethod),
    m_encryptionType(encryptionType),
    QObject(parent)
{
    m_state = InitState;
    m_socket = new QSslSocket(this);

    connect(m_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(m_socket,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(m_socket,SIGNAL(disconnected()),this,SLOT(disconnected()));
}


void SmtpClient::connectToHost()
{
    switch (m_encryptionType) {
    case EncryptionNone:
        m_socket->connectToHost(m_host, m_port);
        break;
    case EncryptionSSL:
        m_socket->connectToHostEncrypted(m_host, m_port);
        break;
    case EncryptionTLS:
        m_socket->connectToHost(m_host,m_port);
        break;
    default:
        break;
    }
}

void SmtpClient::connected()
{
//    qDebug() << "connected to" << m_host;
//    qDebug() << "-----------------------";
}

void SmtpClient::readData()
{
    QString response;
    QString responseLine;

    while(m_socket->canReadLine() && responseLine[3] != ' '){
        responseLine = m_socket->readLine();
        response.append(responseLine);
    }
    responseLine.truncate( 3 );

//    qDebug() << "---------------------------------------------";
//    qDebug() << "Server code:" <<  responseLine;
//    qDebug() << "---------------------------------------------";
//    qDebug() << "Server data: " << response;
//    qDebug() << "---------------------------------------------";

    switch (m_state) {
    case InitState:
        if(responseLine == "220"){
            qDebug() << "Init";
            send("EHLO localhost");
            if(m_encryptionType == EncryptionNone){
                m_state = AuthentificationState;
                break;
            }
            if(m_encryptionType == EncryptionSSL){
                m_state = HandShakeState;
                break;
            }
            if(m_encryptionType == EncryptionTLS){
                m_state = StartTlsState;
                break;
            }
        }
        break;
    case HandShakeState:
        if(responseLine == "250"){
            qDebug() << "Handshake";
            m_socket->startClientEncryption();
            send("EHLO localhost");
            m_state = AuthentificationState;
        }
        if(responseLine == "220"){
            qDebug() << "TLS Handshake";
            m_socket->startClientEncryption();
            send("EHLO localhost");
            m_state = AuthentificationState;
        }
        break;
    case StartTlsState:
        if(responseLine == "250"){
            send("STARTTLS");
            m_state = HandShakeState;
        }
        break;
    case AuthentificationState:
        if(responseLine == "250"){
            qDebug() << "Autentificate";
            if(m_authMethod == AuthLogin){
                send("AUTH LOGIN");
                m_state = UserState;
                break;
            }
            if(m_authMethod == AuthPlain){
                send("AUTH PLAIN " + QByteArray().append((char) 0).append(m_user).append((char) 0).append(m_password).toBase64());
                m_state = MailState;
                break;
            }
        }
        break;
    case UserState:
        if(responseLine == "334"){
            send(QByteArray().append(m_user).toBase64());
            m_state = PasswordState;
        }
        break;
    case PasswordState:
        if(responseLine == "334"){
            send(QByteArray().append(m_password).toBase64());
            m_state = MailState;
        }
        break;
    case MailState:
        if(responseLine == "235"){
            send("MAIL FROM:<" + m_sender + ">");
            m_state = RcptState;
        }
        break;
    case RcptState:
        if(responseLine == "250"){
            send("RCPT TO:<" + m_rcpt + ">");
            m_state = DataState;
        }
        break;
    case DataState:
        if(responseLine == "250"){
            send("DATA");
            m_state = BodyState;
        }
        break;
    case BodyState:
        if(responseLine == "354"){
            send(m_message + "\r\n.\r\n");
            m_state = QuitState;
        }
        break;
    case QuitState:
        if(responseLine == "250"){
            emit sendMailFinished(true, m_actionId);
//            qDebug() << "--------------------------------------------";
//            qDebug() << "              MAIL SENT!!!!";
//            qDebug() << "--------------------------------------------";
            logout();
            m_state = CloseState;
        }
        break;
    case CloseState:
        if(responseLine == "221"){
            m_socket->close();
        }
        m_socket->close();
        break;
    default:
        //qDebug() << "ERROR: unexpected response from server: " << response;
        emit sendMailFinished(false, m_actionId);
        break;
    }
}

void SmtpClient::disconnected()
{
//    qDebug() << "disconnected from" << m_host;
//    qDebug() << "-----------------------";
}

void SmtpClient::login(const QString &user, const QString &password)
{
    if(!m_socket->isOpen()){
        connectToHost();
    }
   // qDebug() << "Try to login with: " << user << password;
}

void SmtpClient::logout()
{
    send("QUIT");
}

bool SmtpClient::sendMail(const QString &subject, const QString &body, const ActionId &actionId)
{
    m_actionId = actionId;

    // create mail content
    m_state = InitState;
    m_message.clear();

    m_message = "To: " + m_rcpt + "\n";
    m_message.append("From: " + m_sender + "\n");
    m_message.append("Subject: " + subject + "\n");
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

void SmtpClient::setRecipiant(const QString &rcpt)
{
    m_rcpt = rcpt;
}

void SmtpClient::socketError(QAbstractSocket::SocketError error)
{
    qWarning() << "mail socket:" << error << m_socket->errorString();
}

void SmtpClient::send(const QString &data)
{
    //qDebug() << "sending to host:" << data;
    m_socket->write(data.toUtf8() + "\r\n");
    m_socket->flush();
}


