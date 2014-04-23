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

SmtpClient::SmtpClient(const QString &host, int port, SmtpClient::ConnectionType connectionType):
    m_host(host), m_port(port), m_connectionType(connectionType)
{
    m_socket = new QSslSocket(this);

    m_authMethod = AuthLogin;
    m_connectionTimeout = 30000;
    m_responseTimeout = 30000;
    m_sendMessageTimeout = 60000;

    connect(m_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(m_socket,SIGNAL(disconnected()),this,SLOT(disconnected()));

}

bool SmtpClient::connectToHost()
{
    switch (m_connectionType) {
    case TlsConnection:
        m_socket->connectToHostEncrypted(m_host, m_port);
        break;
    case TcpConnection:
        m_socket->connectToHost(m_host,m_port);
        break;
    case SslConnection:
        m_socket->connectToHostEncrypted(m_host, m_port);
        break;
    default:
        break;
    }

    if (!m_socket->waitForConnected(m_connectionTimeout)){
        qWarning() << "Mail TCP connection timeout (5s).";
        return false;
    }

    if(waitForResponse() != 220){
        qWarning() << "ERROR: could not connect.";
        m_socket->close();
        return false;
    }

    send("EHLO localhost");

    if(waitForResponse() != 250){
        qWarning() << "ERROR: could not connect.";
        m_socket->close();
        return false;
    }

    if(m_connectionType == TlsConnection){
        //qDebug() << "start client ecryption";
        m_socket->startClientEncryption();

        if(!m_socket->waitForEncrypted(m_connectionTimeout)){
            qDebug() << "encryption falied...";
            m_socket->close();
            return false;
        }

        send("EHLO localhost");
        if(waitForResponse() != 250){
            qWarning() << "ERROR: could not connect.";
            m_socket->close();
            return false;
        }
    }
    return true;
}

void SmtpClient::connected()
{
    qDebug() << "connected to" << m_host;
    qDebug() << "-----------------------";
}

void SmtpClient::disconnected()
{
    qDebug() << "disconnected from" << m_host;
    qDebug() << "-----------------------";
}

bool SmtpClient::login(const QString &user, const QString &password, SmtpClient::AuthMethod method)
{

    if(!connectToHost()){
        m_socket->disconnect();
        return false;
    }

    qDebug() << "Try to login with: " << user << password;

    if(method == AuthPlain){
        send("AUTH PLAIN " + QByteArray().append((char) 0).append(user).append((char) 0).append(password).toBase64());
        if(waitForResponse() != 235){
            qWarning() << "ERROR: could not login AUTH PLAIN";
            m_socket->close();
            return false;
        }
    }else if(method == AuthLogin){
        send("AUTH LOGIN");
        if(waitForResponse() != 334){
            qWarning() << "ERROR: could not login AUTH LOGIN";
            m_socket->close();
            return false;
        }

        send(QByteArray().append(user).toBase64());
        if(waitForResponse() != 334){
            qWarning() << "ERROR: could not login AUTH LOGIN...wrong username or password";
            m_socket->close();
            return false;
        }

        send(QByteArray().append(password).toBase64());
        if(waitForResponse() != 235){
            qWarning() << "ERROR: could not login AUTH LOGIN...wrong username or password";
            m_socket->close();
            return false;
        }
    }
    qDebug() << "login....OK";
    qDebug() << "-----------------------";
    return true;
}

bool SmtpClient::logout()
{
    send("QUIT");
    if(waitForResponse() != 221){
        qWarning() << "ERROR: could not QUIT connection.";
    }

    m_socket->close();
    return true;
}

int SmtpClient::waitForResponse()
{
    while(true){
        if(!m_socket->waitForReadyRead(m_responseTimeout)){
            qWarning() << "ERROR: mail TCP response timeout.";
            return -1;
        }

        qDebug() << "-----------------------";
        QString responseText;
        QString responseLine;
        while (m_socket->canReadLine() && responseLine[3] != ' ') {
            // Save the server's response
            responseLine = m_socket->readLine();
            responseText.append(responseLine);
        }
        int responseCode = responseText.left(3).toInt();
        qDebug() << responseText;
        qDebug() << "-----------------------";
        return responseCode;
    }
}

bool SmtpClient::sendMail(const QString &from, const QString &to, const QString &subject, const QString &body)
{

    send("MAIL FROM:<" + from + ">");
    if(waitForResponse() != 250){
        qWarning() << "ERROR: could not send mail.";
        return false;
    }

    send("RCPT TO:<" + to + ">");
    if(waitForResponse() != 250){
        qWarning() << "ERROR: could not send mail.";
        return false;
    }

    send("DATA");
    if(waitForResponse() != 354){
        qWarning() << "ERROR: could not send mail.";
        return false;
    }

    // mail
    QString message;
    message = "To: " + to + "\n";
    message.append("From: " + from + "\n");
    message.append("Subject: " + subject + "\n");
    message.append(body);
    message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );
    message.replace( QString::fromLatin1( "\r\n.\r\n" ),
    QString::fromLatin1( "\r\n..\r\n" ) );

    qDebug() << "sending...\n";
    qDebug() << "--------------------------------------------";
    send(message + "\r\n.");
    if(waitForResponse() != 250){
        qWarning() << "ERROR: could not send mail.";
        return false;
    }

    logout();

    qDebug() << "--------------------------------------------";
    qDebug() << "              MAIL SENT!!!!";
    qDebug() << "--------------------------------------------";
    return true;
}

void SmtpClient::socketError(QAbstractSocket::SocketError error)
{
    qWarning() << "mail socket:" << error << m_socket->errorString();
}

void SmtpClient::send(const QString &data)
{
    qDebug() << "send:" << data;
    m_socket->write(data.toUtf8() + "\r\n");
    m_socket->flush();
    if (m_socket->waitForBytesWritten(m_sendMessageTimeout)){
        qWarning() << "mail TCP send timeout (60s)";
    }
}


