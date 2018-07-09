/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 **
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>

using namespace nymeaserver;

class TestTags: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void addTag_data();
    void addTag();

    void updateTagValue();

    void removeTag();

private:
    QVariantMap createDeviceTag(const QString &deviceId, const QString &appId, const QString &tagId, const QString &value);
    bool compareDeviceTag(const QVariantMap &tag, const QString &deviceId, const QString &appId, const QString &tagId, const QString &value);
    QVariantMap createRuleTag(const QString &ruleId, const QString &appId, const QString &tagId, const QString &value);
    bool comapreRuleTag(const QVariantMap &tag, const QString &ruleId, const QString &appId, const QString &tagId, const QString &value);
};

QVariantMap TestTags::createDeviceTag(const QString &deviceId, const QString &appId, const QString &tagId, const QString &value)
{
    QVariantMap tag;
    tag.insert("deviceId", deviceId);
    tag.insert("appId", appId);
    tag.insert("tagId", tagId);
    tag.insert("value", value);
    return tag;
}

QVariantMap TestTags::createRuleTag(const QString &ruleId, const QString &appId, const QString &tagId, const QString &value)
{
    QVariantMap tag;
    tag.insert("ruleId", ruleId);
    tag.insert("appId", appId);
    tag.insert("tagId", tagId);
    tag.insert("value", value);
    return tag;
}

bool TestTags::compareDeviceTag(const QVariantMap &tag, const QString &deviceId, const QString &appId, const QString &tagId, const QString &value)
{
    return tag.value("deviceId").toString() == deviceId &&
            tag.value("appId").toString() == appId &&
            tag.value("tagId").toString() == tagId &&
            tag.value("value").toString() == value;
}
void TestTags::addTag_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<QString>("appId");
    QTest::addColumn<QString>("tagId");
    QTest::addColumn<QString>("value");
    QTest::addColumn<TagsStorage::TagError>("expectedError");

    QTest::newRow("tagDevice") << m_mockDeviceId << "testtags" << "favorites" << "1" << TagsStorage::TagErrorNoError;
    QTest::newRow("invalidDevice") << DeviceId::createDeviceId() << "testtags" << "favorites" << "1" << TagsStorage::TagErrorDeviceNotFound;
}

void TestTags::addTag()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(QString, appId);
    QFETCH(QString, tagId);
    QFETCH(QString, value);
    QFETCH(TagsStorage::TagError, expectedError);

    // enable notificartions
    QCOMPARE(enableNotifications(), true);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Create a tag;
    QVariantMap params;
    params.insert("tag", createDeviceTag(deviceId.toString(), appId, tagId, value));
    QVariant response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, expectedError);

    if (expectedError != TagsStorage::TagErrorNoError) {
    // If we expected an error, we can drop out here
        return;
    }

    // Make sure the TagAdded notification is emitted.
    QVariantMap notificationTagMap = checkNotification(clientSpy, "Tags.TagAdded").toMap().value("params").toMap().value("tag").toMap();
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareDeviceTag(notificationTagMap, deviceId.toString(), appId, tagId, value), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());

    // Try getting the tag via GetTag
    params.clear();
    params.insert("deviceId", deviceId.toString());
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    QVariantList tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareDeviceTag(tagsList.first().toMap(), deviceId.toString(), appId, tagId, value), "Fetched tag isn't matching the one we added");
}

void TestTags::updateTagValue()
{
    // enable notificartions
    QCOMPARE(enableNotifications(), true);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QString deviceId = m_mockDeviceId.toString();
    QString appId = "testtags";
    QString tagId = "changedNotificationTag";

    // Create a Tag
    QVariantMap params;
    params.insert("tag", createDeviceTag(deviceId, appId, tagId, "1"));
    QVariant response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagAdded notification
    QVariantMap notificationTagMap = checkNotification(clientSpy, "Tags.TagAdded").toMap().value("params").toMap().value("tag").toMap();
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareDeviceTag(notificationTagMap, deviceId, appId, tagId, "1"), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());
    clientSpy.clear();

    // Try getting the changed tag via GetTag
    params.clear();
    params.insert("deviceId", deviceId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    QVariantList tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareDeviceTag(tagsList.first().toMap(), deviceId, appId, tagId, "1"), "Fetched tag isn't matching the one we added");

    // Now update the tag
    params.clear();
    params.insert("tag", createDeviceTag(deviceId, appId, tagId, "2"));
    response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagAdded notification
    notificationTagMap = checkNotification(clientSpy, "Tags.TagValueChanged").toMap().value("params").toMap().value("tag").toMap();
    jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareDeviceTag(notificationTagMap, deviceId, appId, tagId, "2"), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());

    // Try getting the changed tag via GetTag
    params.clear();
    params.insert("deviceId", deviceId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareDeviceTag(tagsList.first().toMap(), deviceId, appId, tagId, "2"), "Fetched tag isn't matching the one we added");
}

void TestTags::removeTag()
{
    // enable notificartions
    QCOMPARE(enableNotifications(), true);

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QString deviceId = m_mockDeviceId.toString();
    QString appId = "testtags";
    QString tagId = "removeTagTest";
    QString value = "1";

    // Create a Tag
    QVariantMap params;
    params.insert("tag", createDeviceTag(deviceId, appId, tagId, value));
    QVariant response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagAdded notification
    QVariantMap notificationTagMap = checkNotification(clientSpy, "Tags.TagAdded").toMap().value("params").toMap().value("tag").toMap();
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareDeviceTag(notificationTagMap, deviceId, appId, tagId, value), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());
    clientSpy.clear();

    // Try getting the tag via GetTag
    params.clear();
    params.insert("deviceId", deviceId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    QVariantList tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareDeviceTag(tagsList.first().toMap(), deviceId, appId, tagId, value), "Fetched tag isn't matching the one we added");

    // Now remove the tag
    params.clear();
    params.insert("tag", createDeviceTag(deviceId, appId, tagId, QString()));
    response = injectAndWait("Tags.RemoveTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagRemoved notification
    notificationTagMap = checkNotification(clientSpy, "Tags.TagRemoved").toMap().value("params").toMap().value("tag").toMap();
    jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareDeviceTag(notificationTagMap, deviceId, appId, tagId, QString()), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());

    // Try getting the tag via GetTag
    params.clear();
    params.insert("deviceId", deviceId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 0);
}

#include "testtags.moc"
QTEST_MAIN(TestTags)
