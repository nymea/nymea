/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include "httpdaemon.h"

#include "plugin/device.h"
#include "plugin/deviceclass.h"
#include "plugin/deviceplugin.h"
#include "types/statetype.h"
#include "extern-plugininfo.h"

#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QUrlQuery>
#include <QRegExp>
#include <QStringList>

HttpDaemon::HttpDaemon(Device *device, DevicePlugin *parent):
    QTcpServer(parent), disabled(false), m_plugin(parent), m_device(device)
{
    listen(QHostAddress::Any, device->paramValue("httpport").toInt());
}

HttpDaemon::~HttpDaemon()
{
    close();
}

void HttpDaemon::incomingConnection(qintptr socket)
{
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

void HttpDaemon::actionExecuted(const ActionTypeId &actionTypeId)
{
    qCDebug(dcMockDevice) << "Log actions executed" << actionTypeId.toString();
    m_actionList.append(qMakePair<ActionTypeId, QDateTime>(actionTypeId, QDateTime::currentDateTime()));
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
        QUrl url("http://foo.bar" + tokens[1]);
        QUrlQuery query(url);
        if (url.path() == "/setstate") {
            qCDebug(dcMockDevice) << "Set state value" << query.queryItems().first().second;
            emit setState(StateTypeId(query.queryItems().first().first), QVariant(query.queryItems().first().second));
        } else if (url.path() == "/generateevent") {
            emit triggerEvent(EventTypeId(query.queryItemValue("eventtypeid")));
        } else if (url.path() == "/actionhistory") {
            qCDebug(dcMockDevice) << "Get action history called";

            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << generateHeader();
            for (int i = 0; i < m_actionList.count(); ++i) {
                os << m_actionList.at(i).first.toString() << '\n';
                qCDebug(dcMockDevice) << "    " << m_actionList.at(i).first.toString();
            }
            socket->close();
            return;
        } else if (url.path() == "/clearactionhistory") {
            qCDebug(dcMockDevice) << "Clear action history";
            m_actionList.clear();
        }
        if (tokens[0] == "GET") {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << generateWebPage();
            socket->close();

            if (socket->state() == QTcpSocket::UnconnectedState) {
                delete socket;
            }
        }
    }
}

void HttpDaemon::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();

    qCDebug(dcMockDevice) << "Connection closed";
}

QString HttpDaemon::generateHeader()
{
    QString contentHeader(
        "HTTP/1.0 200 Ok\r\n"
       "Content-Type: text/html; charset=\"utf-8\"\r\n"
       "\r\n"
    );
    return contentHeader;
}

QString HttpDaemon::generateWebPage()
{
    DeviceClass deviceClass = m_plugin->supportedDevices().first();

    QString body = QString(
    "<html>"
        "<body>"
        "<h1>Mock device Controller</h1>\n"
        "<hr>"
                "<h2>Device Information</h2>"
        "Name: %1<br>"
        "ID: %2<br>"
        "DeviceClass ID: %3<br>").arg(m_device->paramValue("name").toString()).arg(m_device->id().toString()).arg(deviceClass.id().toString());

    body.append("<hr>");
    body.append("<h2>States</h2>");

    body.append("<table>");
    for (int i = 0; i < deviceClass.stateTypes().count(); ++i) {
        body.append("<tr>");
        body.append("<form action=\"/setstate\" method=\"get\">");
        const StateType &stateType = deviceClass.stateTypes().at(i);
        body.append("<td>" + stateType.name() + "</td>");
        body.append(QString("<td><input type='input'' name='%1' value='%2'></td>").arg(stateType.id().toString()).arg(m_device->states().at(i).value().toString()));
        body.append("<td><input type=submit value='Set State'/></td>");
        body.append("</form>");
        body.append("</tr>");
    }
    body.append("</table>");

    body.append("<hr>");
    body.append("<h2>Events</h2>");

    body.append("<table>");
    for (int i = 0; i < deviceClass.eventTypes().count(); ++i) {
        const EventType &eventType = deviceClass.eventTypes().at(i);
        body.append(QString(
        "<tr>"
        "<form action=\"/generateevent\" method=\"get\">"
        "<td>%1<input type='hidden' name='eventtypeid' value='%2'/></td>"
        "<td>").arg(eventType.name()).arg(eventType.id().toString()));
        if (!eventType.name().endsWith(" changed")) {
            body.append("<input type='submit' value='Generate'/>");
        }
        body.append("</td>"
        "</form>"
        "</tr>"
        );
    }
    body.append("</table>");

    body.append("<hr>");
    body.append("<h2>Actions</h2>");

    body.append("<table border=2px>");
    body.append("<tr><td>Name</td><td>Type ID</td><td>Timestamp</td></tr>");
    for (int i = 0; i < m_actionList.count(); ++i) {
        ActionTypeId actionTypeId = ActionTypeId(m_actionList.at(i).first);
        QDateTime timestamp = m_actionList.at(i).second;
        QString actionName;
        foreach (const ActionType &at, deviceClass.actionTypes()) {
            if (at.id() == actionTypeId) {
                actionName = at.name();
                break;
            }
        }
        body.append(QString(
        "<tr>"
        "<td>%1</td>"
        "<td>%2</td>"
        "<td>%3</td>"
        "</tr>"
        ).arg(actionName).arg(actionTypeId.toString()).arg(timestamp.toString()));
    }
    body.append("</table>");


    body.append("</body></html>\n");

    return generateHeader() + body;
}
