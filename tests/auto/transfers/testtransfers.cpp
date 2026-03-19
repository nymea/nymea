// SPDX-License-Identifier: GPL-3.0-or-later

#include "nymeatestbase.h"
#include "nymeacore.h"

#include <QJsonDocument>
#include <QScopedPointer>
#include <QWebSocket>

using namespace nymeaserver;

class TestTransfers : public NymeaTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testUploadDownloadRoundtrip();
    void testStartDownloadRejectsUnknownDownloadId();

public slots:
    void sslErrors(const QList<QSslError> &)
    {
        QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
        if (socket) {
            socket->ignoreSslErrors();
        }
    }

private:
    QWebSocket *openSocket();
    QVariant sendAndWait(QWebSocket *socket, int id, const QString &method, const QVariantMap &params = QVariantMap(), QVariantMap *notification = nullptr);
    QVariantMap waitForNotification(QSignalSpy &spy, const QString &notificationName);
};

void TestTransfers::initTestCase()
{
    NymeaTestBase::initTestCase();

    ServerConfiguration config;
    config.address = "127.0.0.1";
    config.port = 4444;
    config.sslEnabled = true;
    config.authenticationEnabled = false;
    NymeaCore::instance()->configuration()->setWebSocketServerConfiguration(config);
}

void TestTransfers::testUploadDownloadRoundtrip()
{
    QScopedPointer<QWebSocket> apiSocket(openSocket());
    QVERIFY(!apiSocket.isNull());

    QVariant response = sendAndWait(apiSocket.data(), 1, "JSONRPC.Hello");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    QVariantMap params;
    params.insert("namespaces", QStringList() << "Transfers");
    response = sendAndWait(apiSocket.data(), 2, "JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    const QByteArray payload = QByteArray("transfer-roundtrip-") + QByteArray(8192, 'x') + QByteArray("-payload");
    params.clear();
    params.insert("fileName", "roundtrip.bin");
    params.insert("size", payload.size());
    response = sendAndWait(apiSocket.data(), 3, "Transfers.CreateUpload", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    const QVariantMap uploadInfo = response.toMap().value("params").toMap();
    const QString uploadTransferId = uploadInfo.value("transferId").toString();
    const QString uploadTransferToken = uploadInfo.value("transferToken").toString();
    QVERIFY(!uploadTransferId.isEmpty());
    QVERIFY(!uploadTransferToken.isEmpty());

    QScopedPointer<QWebSocket> uploadSocket(openSocket());
    QVERIFY(!uploadSocket.isNull());

    params.clear();
    params.insert("transferId", uploadTransferId);
    params.insert("transferToken", uploadTransferToken);
    response = sendAndWait(uploadSocket.data(), 10, "Transfer.Connect", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("direction").toString(), QString("upload"));

    const QList<QByteArray> chunks = {
        payload.left(payload.size() / 3),
        payload.mid(payload.size() / 3, payload.size() / 3),
        payload.mid((payload.size() / 3) * 2)
    };

    int transferCommandId = 11;
    for (int i = 0; i < chunks.count(); ++i) {
        params.clear();
        params.insert("data", chunks.at(i).toBase64());
        response = sendAndWait(uploadSocket.data(), transferCommandId++, "Transfer.UploadChunk", params);
        QCOMPARE(response.toMap().value("status").toString(), QString("success"));

        QVariantMap keepAliveParams;
        keepAliveParams.insert("sessionId", QString("upload-%1").arg(i));
        const QVariant keepAliveResponse = sendAndWait(apiSocket.data(), 100 + i, "JSONRPC.KeepAlive", keepAliveParams);
        QCOMPARE(keepAliveResponse.toMap().value("status").toString(), QString("success"));
    }

    QSignalSpy apiNotificationSpy(apiSocket.data(), &QWebSocket::textMessageReceived);
    response = sendAndWait(uploadSocket.data(), transferCommandId++, "Transfer.FinishUpload");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    const QVariantMap downloadNotification = waitForNotification(apiNotificationSpy, "Transfers.DownloadAvailable");
    QCOMPARE(downloadNotification.value("notification").toString(), QString("Transfers.DownloadAvailable"));

    const QString downloadId = downloadNotification.value("params").toMap().value("downloadId").toString();
    QVERIFY(!downloadId.isEmpty());

    params.clear();
    params.insert("downloadId", downloadId);
    response = sendAndWait(apiSocket.data(), 200, "Transfers.StartDownload", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    const QVariantMap downloadInfo = response.toMap().value("params").toMap();
    const QString downloadTransferId = downloadInfo.value("transferId").toString();
    const QString downloadTransferToken = downloadInfo.value("transferToken").toString();
    QVERIFY(!downloadTransferId.isEmpty());
    QVERIFY(!downloadTransferToken.isEmpty());

    QScopedPointer<QWebSocket> downloadSocket(openSocket());
    QVERIFY(!downloadSocket.isNull());

    params.clear();
    params.insert("transferId", downloadTransferId);
    params.insert("transferToken", downloadTransferToken);
    response = sendAndWait(downloadSocket.data(), 20, "Transfer.Connect", params);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("direction").toString(), QString("download"));

    QByteArray downloadedPayload;
    bool finished = false;
    int downloadCommandId = 21;
    while (!finished) {
        params.clear();
        params.insert("maxBytes", 2048);
        response = sendAndWait(downloadSocket.data(), downloadCommandId++, "Transfer.RequestChunk", params);
        QCOMPARE(response.toMap().value("status").toString(), QString("success"));
        const QVariantMap chunkParams = response.toMap().value("params").toMap();
        downloadedPayload.append(QByteArray::fromBase64(chunkParams.value("data").toByteArray()));
        finished = chunkParams.value("finished").toBool();
    }

    QCOMPARE(downloadedPayload, payload);
}

void TestTransfers::testStartDownloadRejectsUnknownDownloadId()
{
    QScopedPointer<QWebSocket> apiSocket(openSocket());
    QVERIFY(!apiSocket.isNull());

    QVariant response = sendAndWait(apiSocket.data(), 1, "JSONRPC.Hello");
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    QVariantMap params;
    params.insert("downloadId", QUuid::createUuid().toString(QUuid::WithoutBraces));
    response = sendAndWait(apiSocket.data(), 2, "Transfers.StartDownload", params);

    QCOMPARE(response.toMap().value("status").toString(), QString("error"));
    QCOMPARE(response.toMap().value("error").toString(), QString("Unknown download"));
}

QWebSocket *TestTransfers::openSocket()
{
    QWebSocket *socket = new QWebSocket("nymea transfer tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestTransfers::sslErrors);

    QSignalSpy connectedSpy(socket, &QWebSocket::connected);
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    if (!connectedSpy.wait()) {
        socket->deleteLater();
        return nullptr;
    }

    return socket;
}

QVariant TestTransfers::sendAndWait(QWebSocket *socket, int id, const QString &method, const QVariantMap &params, QVariantMap *notification)
{
    QVariantMap call;
    call.insert("id", id);
    call.insert("method", method);
    if (method.startsWith("JSONRPC.") || method.startsWith("Transfers.")) {
        call.insert("token", m_apiToken);
    }
    if (!params.isEmpty()) {
        call.insert("params", params);
    }

    const QByteArray payload = QJsonDocument::fromVariant(call).toJson(QJsonDocument::Compact);
    QSignalSpy spy(socket, &QWebSocket::textMessageReceived);
    socket->sendTextMessage(QString::fromUtf8(payload));

    while (spy.count() > 0 || spy.wait()) {
        for (int i = 0; i < spy.count(); ++i) {
            QJsonParseError error;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).at(0).toByteArray(), &error);
            if (error.error != QJsonParseError::NoError) {
                continue;
            }

            const QVariantMap response = jsonDoc.toVariant().toMap();
            if (response.value("notification").toString() == "Transfers.DownloadAvailable" && notification) {
                *notification = response;
                continue;
            }

            if (response.value("id").toInt() == id) {
                return response;
            }
        }
        spy.clear();
    }

    return QVariant();
}

QVariantMap TestTransfers::waitForNotification(QSignalSpy &spy, const QString &notificationName)
{
    while (spy.count() > 0 || spy.wait()) {
        for (int i = 0; i < spy.count(); ++i) {
            QJsonParseError error;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).at(0).toByteArray(), &error);
            if (error.error != QJsonParseError::NoError) {
                continue;
            }

            const QVariantMap response = jsonDoc.toVariant().toMap();
            if (response.value("notification").toString() == notificationName) {
                return response;
            }
        }
        spy.clear();
    }

    return QVariantMap();
}

#include "testtransfers.moc"
QTEST_MAIN(TestTransfers)
