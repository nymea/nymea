// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "coaptests.h"

#include <QJsonDocument>

CoapTests::CoapTests(QObject *parent)
    : QObject(parent)
{
    m_coap = new Coap(this);
    m_uploadData = QByteArray("                   GNU GENERAL PUBLIC LICENSE \n"
                              "                    Version 3, 29 June 2007 \n"
                              "\n"
                              "Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>\n"
                              "Everyone is permitted to copy and distribute verbatim copies\n"
                              "of this license document, but changing it is not allowed.\n"
                              "\n"
                              "                         Preamble\n"
                              "\n"
                              "The GNU General Public License is a free, copyleft license for\n"
                              "software and other kinds of works.\n"
                              "\n"
                              "The licenses for most software and other practical works are designed\n"
                              "to take away your freedom to share and change the works.  By contrast,\n"
                              "the GNU General Public License is intended to guarantee your freedom to\n"
                              "share and change all versions of a program--to make sure it remains free\n"
                              "software for all its users.  We, the Free Software Foundation, use the\n"
                              "GNU General Public License for most of our software; it applies also to\n"
                              "any other work released this way by its authors.  You can apply it to\n"
                              "your programs, too.\n"
                              "\n"
                              "When we speak of free software, we are referring to freedom, not\n"
                              "price.  Our General Public Licenses are designed to make sure that you\n"
                              "have the freedom to distribute copies of free software (and charge for\n"
                              "them if you wish), that you receive source code or can get it if you\n"
                              "want it, that you can change the software or use pieces of it in new\n"
                              "free programs, and that you know you can do these things.");
}

void CoapTests::invalidUrl_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("missing backslash") << QUrl("coap:/coap.me");
    QTest::newRow("invalid host") << QUrl("coap://foo.bar");
}

void CoapTests::invalidUrl()
{
    QFETCH(QUrl, url);

    CoapRequest request(url);
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->error(), CoapReply::HostNotFoundError);
}

void CoapTests::invalidScheme()
{
    CoapRequest request(QUrl("http://coap.me"));

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));

    // Get
    CoapReply *reply = m_coap->get(request);
    spy.wait(1000);

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() == 0, "Got a response.");
    QCOMPARE(reply->error(), CoapReply::InvalidUrlSchemeError);

    reply->deleteLater();

    // Post
    spy.clear();
    reply = m_coap->post(request);
    spy.wait(1000);

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() == 0, "Got a response.");
    QCOMPARE(reply->error(), CoapReply::InvalidUrlSchemeError);

    // Put
    spy.clear();
    reply = m_coap->put(request);
    spy.wait(1000);

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() == 0, "Got a response.");
    QCOMPARE(reply->error(), CoapReply::InvalidUrlSchemeError);

    // Delete
    spy.clear();
    reply = m_coap->deleteResource(request);
    spy.wait(1000);

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() == 0, "Got a response.");
    QCOMPARE(reply->error(), CoapReply::InvalidUrlSchemeError);

    // Ping
    spy.clear();
    reply = m_coap->ping(request);
    spy.wait(1000);

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() == 0, "Got a response.");
    QCOMPARE(reply->error(), CoapReply::InvalidUrlSchemeError);
}

void CoapTests::ping()
{
    CoapRequest request;
    request.setUrl(QUrl("coap://coap.me/"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->ping(request);
    spy.wait();

    QVERIFY2(reply->isFinished(), "Reply not finished.");
    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->statusCode(), CoapPdu::Empty);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY(reply->payload().isEmpty());
    reply->deleteLater();
}

void CoapTests::hello()
{
    CoapRequest request(QUrl("coap://coap.me/hello"));
    request.setMessageType(CoapPdu::Confirmable);

    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    qDebug() << reply->isRunning() << reply->errorString();
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "world", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::broken()
{
    CoapRequest request(QUrl("coap://coap.me:5683/broken"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::InternalServerError);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QVERIFY2(reply->payload() == "Oops: broken", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::query()
{
    CoapRequest request(QUrl("coap://coap.me/query?nymea=awesome"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "You asked me about: nymea=awesome", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::subPath()
{
    CoapRequest request(QUrl("coap://coap.me/path/sub1"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "TD_CORE_COAP_09 sub1", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::extendedOptionLength()
{
    CoapRequest request(QUrl("coap://coap.me:5683/123412341234123412341234"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "very long resource name", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::specialCharacters()
{
    CoapRequest request(QUrl("coap://coap.me:5683/blåbærsyltetøy"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "Übergrößenträger = 特大の人 = 超大航母", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::extendedDelta_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("weird33") << QUrl("coap://coap.me/weird33");
    QTest::newRow("weird44") << QUrl("coap://coap.me/weird44");
    QTest::newRow("weird55") << QUrl("coap://coap.me/weird55");
    QTest::newRow("weird333") << QUrl("coap://coap.me/weird333");
    QTest::newRow("weird3333") << QUrl("coap://coap.me/weird3333");
    QTest::newRow("weird33333") << QUrl("coap://coap.me/weird33333");
}

void CoapTests::extendedDelta()
{
    QFETCH(QUrl, url);

    CoapRequest request(url);
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload().startsWith("resource with option"), "Invalid payload");
    reply->deleteLater();
}

void CoapTests::secret()
{
    CoapRequest request(QUrl("coap://coap.me/secret"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Unauthorized);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "Not authorized", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::separated()
{
    CoapRequest request(QUrl("coap://coap.me:5683/separate"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait(10000);

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "That took a long time", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::deleteResource()
{
    CoapRequest request(QUrl("coap://coap.me:5683/validate"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->deleteResource(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Deleted);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "DELETE OK", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::post()
{
    CoapRequest request(QUrl("coap://coap.me:5683/validate"));
    request.setContentType();
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->post(request, "nymea is awesome");
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Created);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "POST OK", "Invalid payload");
    reply->deleteLater();
}

void CoapTests::put()
{
    CoapRequest request(QUrl("coap://coap.me:5683/validate"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->put(request, "nymea is awesome");
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Changed);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload() == "PUT OK", "Invalid payload");

    reply->deleteLater();
}

void CoapTests::jsonMessage()
{
    CoapRequest request(QUrl("coap://coap.me:5683/5"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::ApplicationJson);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->payload(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    qDebug() << "====================================";
    qDebug() << jsonDoc.toJson();

    reply->deleteLater();
}

void CoapTests::largeDownload()
{
    CoapRequest request(QUrl("coap://coap.me:5683/large"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait(20000);

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QVERIFY2(reply->payload().size() == 1700, "Invalid payload size.");

    reply->deleteLater();
}

void CoapTests::largeCreate()
{
    CoapRequest request(QUrl("coap://coap.me:5683/large-create"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));

    CoapReply *reply = m_coap->post(request, m_uploadData);
    spy.wait(20000);

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Created);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);

    // clean up
    reply->deleteLater();
    spy.clear();

    // check if the upload was really successful
    reply = m_coap->get(request);
    spy.wait(20000);

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QCOMPARE(reply->payload(), m_uploadData);

    reply->deleteLater();
}

void CoapTests::largeUpdate()
{
    CoapRequest request(QUrl("coap://coap.me:5683/large-update"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));

    CoapReply *reply = m_coap->put(request, m_uploadData);
    spy.wait(20000);

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Changed);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);

    // clean up
    reply->deleteLater();
    spy.clear();

    // check if the upload was successful
    reply = m_coap->get(request);
    spy.wait(20000);

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::TextPlain);
    QCOMPARE(reply->error(), CoapReply::NoError);
    QCOMPARE(reply->payload(), m_uploadData);

    reply->deleteLater();
}

void CoapTests::multipleCalls()
{
    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));

    QList<CoapReply *> replies;

    replies.append(m_coap->get(CoapRequest(QUrl("coap://coap.me:5683/separate"))));
    replies.append(m_coap->ping(CoapRequest(QUrl("coap://coap.me"))));
    replies.append(m_coap->get(CoapRequest(QUrl("coap://coap.me:5683/large"))));
    replies.append(m_coap->get(CoapRequest(QUrl("coap://coap.me"))));
    spy.wait(10000);
    spy.wait();
    spy.wait(10000);
    spy.wait();

    QVERIFY2(spy.count() == 4, "Did not get all responses.");
    spy.clear();

    foreach (CoapReply *reply, replies) {
        QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
        QCOMPARE(reply->error(), CoapReply::NoError);
        reply->deleteLater();
    }

    qDeleteAll(replies);
}

void CoapTests::coreLinkParser()
{
    CoapRequest request(QUrl("coap://coap.me/.well-known/core"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    CoapReply *reply = m_coap->get(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->contentType(), CoapPdu::ApplicationLink);
    QCOMPARE(reply->error(), CoapReply::NoError);

    CoreLinkParser parser(reply->payload());
    QCOMPARE(parser.links().count(), 28);

    foreach (const CoreLink &link, parser.links()) {
        qDebug() << link;
    }

    reply->deleteLater();
}

void CoapTests::observeResource()
{
    CoapRequest request(QUrl("coap://vs0.inf.ethz.ch/obs"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    QSignalSpy notificationSpy(m_coap, SIGNAL(notificationReceived(CoapObserveResource, int, QByteArray)));

    CoapReply *reply = m_coap->enableResourceNotifications(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->error(), CoapReply::NoError);
    reply->deleteLater();

    notificationSpy.wait(6000);
    QVERIFY2(notificationSpy.count() > 0, "Did not get a notification.");
    qDebug() << notificationSpy.first();
    m_coap->disableNotifications(request);
    QTest::qWait(5000);
}

void CoapTests::observeLargeResource()
{
    CoapRequest request(QUrl("coap://vs0.inf.ethz.ch/obs-large"));
    qDebug() << request.url().toString();

    QSignalSpy spy(m_coap, SIGNAL(replyFinished(CoapReply *)));
    QSignalSpy notificationSpy(m_coap, SIGNAL(notificationReceived(CoapObserveResource, int, QByteArray)));

    CoapReply *reply = m_coap->enableResourceNotifications(request);
    spy.wait();

    QVERIFY2(spy.count() > 0, "Did not get a response.");
    QCOMPARE(reply->messageType(), CoapPdu::Acknowledgement);
    QCOMPARE(reply->statusCode(), CoapPdu::Content);
    QCOMPARE(reply->error(), CoapReply::NoError);
    reply->deleteLater();

    notificationSpy.wait(6000);
    QVERIFY2(notificationSpy.count() > 0, "Did not get a notification.");
    qDebug() << notificationSpy.first();
    m_coap->disableNotifications(request);
    QTest::qWait(5000);
}

QTEST_MAIN(CoapTests)
