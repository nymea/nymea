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

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"
#include "webserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QMetaType>
#include <QByteArray>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>

using namespace guhserver;

class TestWebSocketServer: public GuhTestBase
{
    Q_OBJECT

private slots:
    void testHandshake();

    void pingTest();

    void introspect();

private:

};


void TestWebSocketServer::testHandshake()
{
    QWebSocket *socket = new QWebSocket("guh tests", QWebSocketProtocol::Version13);
    QSignalSpy spy(socket, SIGNAL(textMessageReceived(QString)));
    socket->open(QUrl(QStringLiteral("ws://localhost:4444")));
    spy.wait();
    QVERIFY2(spy.count() > 0, "Did not get the handshake message upon connect.");
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().first().toByteArray());
    QVariantMap handShake = jsonDoc.toVariant().toMap();

    QString guhVersionString(GUH_VERSION_STRING);
    QString jsonProtocolVersionString(JSON_PROTOCOL_VERSION);
    QCOMPARE(handShake.value("version").toString(), guhVersionString);
    QCOMPARE(handShake.value("protocol version").toString(), jsonProtocolVersionString);

    socket->close();
    socket->deleteLater();
}

void TestWebSocketServer::pingTest()
{
    QWebSocket *socket = new QWebSocket("guh tests", QWebSocketProtocol::Version13);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(QUrl(QStringLiteral("ws://localhost:4444")));
    spyConnection.wait();
    QVERIFY2(spyConnection.count() > 0, "not connected");

    QSignalSpy spyPong(socket, SIGNAL(pong(quint64,QByteArray)));
    socket->ping("hallo");
    spyPong.wait();
    QVERIFY2(spyPong.count() > 0, "no pong");
    qDebug() << "ping response" << spyPong.first().at(0) << spyPong.first().at(1).toString();
}

void TestWebSocketServer::introspect()
{

}

#include "testwebsocketserver.moc"
QTEST_MAIN(TestWebSocketServer)
