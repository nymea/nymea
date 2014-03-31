#include "httpdaemon.h"

#include "device.h"

#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QUrlQuery>

HttpDaemon::HttpDaemon(Device *device, QObject *parent):
    QTcpServer(parent), disabled(false), m_device(device)
{
    listen(QHostAddress::LocalHost, device->params().value("httpport").toInt());
}

void HttpDaemon::incomingConnection(qintptr socket)
{
    qDebug() << "incoming connection";
    if (disabled)
        return;

    // When a new client connects, the server constructs a QTcpSocket and all
    // communication with the client is done over this QTcpSocket. QTcpSocket
    // works asynchronously, this means that all the communication is done
    // in the two slots readClient() and discardClient().
    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);

}

void HttpDaemon::pause()
{
    disabled = true;
}

void HttpDaemon::resume()
{
    disabled = false;
}

void HttpDaemon::readClient()
{
    if (disabled)
        return;

    // This slot is called when the client sent data to the server. The
    // server looks if it was a get request and sends a very simple HTML
    // document back.
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        QStringList tokens = QString(data).split(QRegExp("[ \r\n][ \r\n]*"));
        qDebug() << "incoming data" << tokens[1];
        if (tokens[1].contains('?')) {
            QUrlQuery query(QUrl("http://foo.bar" + tokens[1]));
            qDebug() << "query is" << query.queryItemValue("eventid");
            emit triggerEvent(query.queryItemValue("eventid").toInt());
        }
        if (tokens[0] == "GET") {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << QString("HTTP/1.0 200 Ok\r\n"
                "Content-Type: text/html; charset=\"utf-8\"\r\n"
                "\r\n"
                "<html>"
                  "<body>"
                  "<h1>Mock device</h1>\n"
                  "Name: %1<br>"
                  "ID: %2"
                  "<form action=\"/\" method=\"get\">"
                  "<input type='hidden'' name='eventid' value='1'>"
                  "<input type='submit' value='Generate event1'/>"
                  "</form>"
                  "</body>"
                "</html>\n").arg(m_device->name()).arg(m_device->id().toString());
            socket->close();

            qDebug() << "Wrote to client";

            if (socket->state() == QTcpSocket::UnconnectedState) {
                delete socket;
                qDebug() << "Connection closed";
            }
        }
    }
}

void HttpDaemon::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();

    qDebug() << "Connection closed";
}
