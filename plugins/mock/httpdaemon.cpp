/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "httpdaemon.h"

#include "integrations/thing.h"
#include "integrations/integrationplugin.h"
#include "types/thingclass.h"
#include "types/statetype.h"
#include "extern-plugininfo.h"

#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QUrlQuery>
#include <QRegExp>
#include <QStringList>

HttpDaemon::HttpDaemon(Thing *thing, IntegrationPlugin *parent):
    QTcpServer(parent), disabled(false), m_plugin(parent), m_thing(thing)
{
    QHash<ThingClassId, ParamTypeId> portMap;
    portMap.insert(mockThingClassId, mockThingHttpportParamTypeId);
    portMap.insert(autoMockThingClassId, autoMockThingHttpportParamTypeId);
    listen(QHostAddress::Any, thing->paramValue(portMap.value(thing->thingClassId())).toInt());
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
            StateTypeId stateTypeId = StateTypeId(query.queryItems().first().first);
            QVariant stateValue = query.queryItems().first().second;
            if (stateTypeId == mockBoolStateTypeId || stateTypeId == mockBatteryCriticalStateTypeId) {
                stateValue.convert(QVariant::Bool);
            } else if (stateTypeId == mockIntStateTypeId) {
                stateValue.convert(QVariant::Int);
            } else if (stateTypeId == mockSignalStrengthStateTypeId) {
                stateValue.convert(QVariant::UInt);
            } else if (stateTypeId == mockDoubleStateTypeId) {
                stateValue.convert(QVariant::Double);
            }
            qCDebug(dcMock()) << "Setting state value" << stateValue;
            emit setState(stateTypeId, stateValue);
        } else if (url.path() == "/generateevent") {
            qCDebug(dcMock()) << "Generate event called" << url.query();
            QList<QPair<QString, QString>> queryItems = query.queryItems();
            ParamList params;
            for (int i = 0; i < queryItems.count(); i++) {
                QPair<QString, QString> item = queryItems.at(i);
                if (item.first != "eventtypeid") {
                    params.append(Param(ParamTypeId(item.first), item.second));
                }
            }
            emit triggerEvent(EventTypeId(query.queryItemValue("eventtypeid")), params);
        } else if (url.path() == "/actionhistory") {
            qCDebug(dcMock()) << "Get action history called";

            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << generateHeader();
            for (int i = 0; i < m_actionList.count(); ++i) {
                os << m_actionList.at(i).first.toString() << '\n';
                qCDebug(dcMock()) << "    " << m_actionList.at(i).first.toString();
            }
            socket->close();
            return;
        } else if (url.path() == "/clearactionhistory") {
            qCDebug(dcMock()) << "Clear action history";
            m_actionList.clear();
        } else if (url.path() == "/disappear") {
            qCDebug(dcMock()) << "Should disappear";
            emit disappear();
        } else if (url.path() == "/reconfigureautodevice") {
            qCDebug(dcMock()) << "Reconfigure auto device";
            emit reconfigureAutodevice();
        }

        if (tokens[0] == "GET") {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << generateWebPage();
            socket->close();

            if (socket->state() == QTcpSocket::UnconnectedState)
                delete socket;
        }
    }
}

void HttpDaemon::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
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
    ThingClass thingClass;
    foreach (const ThingClass &tc, m_plugin->supportedThings()) {
        if (tc.id() == m_thing->thingClassId()) {
            thingClass = tc;
        }
    }
    Q_ASSERT(thingClass.isValid());

    QString body = QString(
    "<html>"
        "<body>"
        "<h1>Mock thing controller</h1>\n"
        "<hr>"
                "<h2>Thing Information</h2>"
        "Name: %1<br>"
        "ID: %2<br>"
        "ThingClass ID: %3<br>").arg(m_thing->name()).arg(m_thing->id().toString()).arg(thingClass.id().toString());

    body.append("<hr>");
    body.append("<h2>States</h2>");

    body.append("<table>");
    for (int i = 0; i < thingClass.stateTypes().count(); ++i) {
        body.append("<tr>");
        body.append("<form action=\"/setstate\" method=\"get\">");
        StateType stateType = thingClass.stateTypes().at(i);
        body.append("<td>" + stateType.name() + "</td>");
        body.append(QString("<td><input type='input'' name='%1' value='%2'></td>").arg(stateType.id().toString()).arg(m_thing->states().at(i).value().toString()));
        body.append("<td><input type=submit value='Set State'/></td>");
        body.append("</form>");
        body.append("</tr>");
    }
    body.append("</table>");

    body.append("<hr>");
    body.append("<h2>Events</h2>");

    body.append("<table>");
    for (int i = 0; i < thingClass.eventTypes().count(); ++i) {
        EventType eventType = thingClass.eventTypes().at(i);
        body.append(QString(
        "<tr>"
        "<form action=\"/generateevent\" method=\"get\">"
        "<td>%1<input type='hidden' name='eventtypeid' value='%2'/></td>"
        "<td>").arg(eventType.name()).arg(eventType.id().toString()));
        if (!eventType.displayName().endsWith(" changed")) {
            body.append(QStringLiteral("<input type='submit' value='Generate'/>"));
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
        foreach (const ActionType &at, thingClass.actionTypes()) {
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
