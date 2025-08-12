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

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "version.h"

#include <QWebSocket>

using namespace nymeaserver;

class TestWebSocketServer: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();

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


void TestWebSocketServer::initTestCase()
{
    NymeaTestBase::initTestCase();

    ServerConfiguration config;
    foreach (const ServerConfiguration &c, NymeaCore::instance()->configuration()->webSocketServerConfigurations()) {
        if (c.port == 4444 && (QHostAddress(c.address) == QHostAddress("127.0.0.1") || QHostAddress(c.address) == QHostAddress("0.0.0.0"))) {
            qDebug() << "Already have a websocketserver listening on 127.0.0.1:4444";
            config = c;
            break;
        }
    }

    qDebug() << "Creating new websocketserver instance on 127.0.0.1:4444";
    config.address = "127.0.0.1";
    config.port = 4444;
    config.sslEnabled = true;
    config.authenticationEnabled = false;
    NymeaCore::instance()->configuration()->setWebSocketServerConfiguration(config);

}

void TestWebSocketServer::testHandshake()
{
    QWebSocket *socket = new QWebSocket("nymea tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);

    QSignalSpy connectedSpy(socket, &QWebSocket::connected);
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    connectedSpy.wait();

    QSignalSpy spy(socket, &QWebSocket::textMessageReceived);
    socket->sendTextMessage("{\"id\":0, \"method\": \"JSONRPC.Hello\"}");

    spy.wait();
    QVERIFY2(spy.count() > 0, "Did not get the handshake message upon connect.");
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().first().toByteArray());
    QVariantMap handShake = jsonDoc.toVariant().toMap();

    QString nymeaVersionString(NYMEA_VERSION_STRING);
    QString jsonProtocolVersionString(JSON_PROTOCOL_VERSION);
    QCOMPARE(handShake.value("params").toMap().value("version").toString(), nymeaVersionString);
    QCOMPARE(handShake.value("params").toMap().value("protocol version").toString(), jsonProtocolVersionString);

    socket->close();
    socket->deleteLater();
}

void TestWebSocketServer::pingTest()
{
    QWebSocket *socket = new QWebSocket("nymea tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spyConnection(socket, &QWebSocket::connected);
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    spyConnection.wait();
    QVERIFY2(spyConnection.count() > 0, "not connected");

    QSignalSpy spyPong(socket, &QWebSocket::pong);
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
        QVERIFY2(response.toMap().value("status").toString() == "success", "Call wasn't parsed correctly by nymea.");
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

    QWebSocket *socket = new QWebSocket("nymea tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spyConnection(socket, &QWebSocket::connected);
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy spy(socket, &QWebSocket::textMessageReceived);
    socket->sendTextMessage("{\"id\":0, \"method\": \"JSONRPC.Hello\"}");
    spy.wait();

    spy.clear();
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
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
    QWebSocket *socket = new QWebSocket("nymea tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestWebSocketServer::sslErrors);
    QSignalSpy spyConnection(socket, &QWebSocket::connected);
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy spy(socket, &QWebSocket::textMessageReceived);

    socket->sendTextMessage("{\"id\":0, \"method\": \"JSONRPC.Hello\"}");
    spy.wait();

    spy.clear();
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
