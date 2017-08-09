/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
#include <QJsonDocument>

#include <QWebSocket>

using namespace guhserver;

class TestWebSocketServer: public GuhTestBase
{
    Q_OBJECT

private slots:
    void testHandshake();

    void pingTest();

    void testBasicCall_data();
    void testBasicCall();

    void introspect();

public slots:
    void sslErrors(const QList<QSslError> &) {
        QWebSocket *socket = static_cast<QWebSocket*>(sender());
        socket->ignoreSslErrors();
    }

private:
    int m_socketCommandId;

    QVariant injectSocketAndWait(const QString &method, const QVariantMap &params = QVariantMap());
    QVariant injectSocketData(const QByteArray &data);
};


void TestWebSocketServer::testHandshake()
{
    QWebSocket *socket = new QWebSocket("guh tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spy(socket, SIGNAL(textMessageReceived(QString)));
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
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
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    spyConnection.wait();
    QVERIFY2(spyConnection.count() > 0, "not connected");

    QSignalSpy spyPong(socket, SIGNAL(pong(quint64,QByteArray)));
    socket->ping("hallo");
    spyPong.wait();
    QVERIFY2(spyPong.count() > 0, "no pong");
    qDebug() << "ping response" << spyPong.first().at(0) << spyPong.first().at(1).toString();
    socket->close();
    socket->deleteLater();
}

void TestWebSocketServer::testBasicCall_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid call") << QByteArray("{\"id\":42, \"method\":\"JSONRPC.Introspect\"}") << true;
    QTest::newRow("missing id") << QByteArray("{\"method\":\"JSONRPC.Introspect\"}")<< false;
    QTest::newRow("missing method") << QByteArray("{\"id\":42}") << false;
    QTest::newRow("borked") << QByteArray("{\"id\":42, \"method\":\"JSO")<< false;
    QTest::newRow("invalid function") << QByteArray("{\"id\":42, \"method\":\"JSONRPC.Foobar\"}") << false;
    QTest::newRow("invalid namespace") << QByteArray("{\"id\":42, \"method\":\"FOO.Introspect\"}") << false;
    QTest::newRow("missing dot") << QByteArray("{\"id\":42, \"method\":\"JSONRPCIntrospect\"}") << false;
    QTest::newRow("invalid params") << QByteArray("{\"id\":42, \"method\":\"JSONRPC.Introspect\", \"params\":{\"törööö\":\"chooo-chooo\"}}") << false;
}

void TestWebSocketServer::testBasicCall()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, valid);

    QVariant response = injectSocketData(data);
    if (valid)
        QVERIFY2(response.toMap().value("status").toString() == "success", "Call wasn't parsed correctly by guh.");
}


void TestWebSocketServer::introspect()
{
    QVariant response = injectSocketAndWait("JSONRPC.Introspect");

    QVariantMap methods = response.toMap().value("params").toMap().value("methods").toMap();
    QVariantMap notifications = response.toMap().value("params").toMap().value("notifications").toMap();
    QVariantMap types = response.toMap().value("params").toMap().value("types").toMap();

    QVERIFY2(methods.count() > 0, "No methods in Introspect response!");
    QVERIFY2(notifications.count() > 0, "No notifications in Introspect response!");
    QVERIFY2(types.count() > 0, "No types in Introspect response!");

}

QVariant TestWebSocketServer::injectSocketAndWait(const QString &method, const QVariantMap &params)
{
    QVariantMap call;
    call.insert("id", m_socketCommandId);
    call.insert("method", method);
    call.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(call);

    QWebSocket *socket = new QWebSocket("guh tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy spy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(jsonDoc.toJson()));
    spy.wait();

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response it a valid JSON string
        QJsonParseError error;
        jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }
        QVariantMap response = jsonDoc.toVariant().toMap();

        // skip notifications
        if (response.contains("notification"))
            continue;

        if (response.value("id").toInt() == m_socketCommandId) {
            m_socketCommandId++;
            return jsonDoc.toVariant();
        }
    }
    m_socketCommandId++;
    return QVariant();
}

QVariant TestWebSocketServer::injectSocketData(const QByteArray &data)
{
    QWebSocket *socket = new QWebSocket("guh tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy spy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(data));
    spy.wait();

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response it a valid JSON string
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }
        return jsonDoc.toVariant();
    }
    m_socketCommandId++;
    return QVariant();
}

#include "testwebsocketserver.moc"
QTEST_MAIN(TestWebSocketServer)
