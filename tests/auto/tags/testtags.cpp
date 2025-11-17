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

#include "nymeatestbase.h"
#include "servers/mocktcpserver.h"
#include "tagging/tagsstorage.h"
#include "../plugins/mock/extern-plugininfo.h"

using namespace nymeaserver;

class TestTags: public NymeaTestBase
{
    Q_OBJECT

private:
    inline void verifyTagError(const QVariant &response, TagsStorage::TagError error = TagsStorage::TagErrorNoError) {
        verifyError(response, "tagError", enumValueName(error));
    }

private slots:
    void addTag_data();
    void addTag();

    void updateTagValue();

    void removeTag();

    void ruleTagIsRemovedOnRuleRemove();

private:
    QVariantMap createThingTag(const QString &thingId, const QString &appId, const QString &tagId, const QString &value);
    bool compareThingTag(const QVariantMap &tag, const QUuid &thingId, const QString &appId, const QString &tagId, const QString &value);
    QVariantMap createRuleTag(const QString &ruleId, const QString &appId, const QString &tagId, const QString &value);
    bool comapreRuleTag(const QVariantMap &tag, const QString &ruleId, const QString &appId, const QString &tagId, const QString &value);
};

QVariantMap TestTags::createThingTag(const QString &thingId, const QString &appId, const QString &tagId, const QString &value)
{
    QVariantMap tag;
    tag.insert("thingId", thingId);
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

bool TestTags::compareThingTag(const QVariantMap &tag, const QUuid &thingId, const QString &appId, const QString &tagId, const QString &value)
{
    return tag.value("thingId").toUuid() == thingId &&
            tag.value("appId").toString() == appId &&
            tag.value("tagId").toString() == tagId &&
            tag.value("value").toString() == value;
}
void TestTags::addTag_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<QString>("appId");
    QTest::addColumn<QString>("tagId");
    QTest::addColumn<QString>("value");
    QTest::addColumn<TagsStorage::TagError>("expectedError");

    QTest::newRow("tagThing") << m_mockThingId << "testtags" << "favorites" << "1" << TagsStorage::TagErrorNoError;
    QTest::newRow("invalidThing") << ThingId::createThingId() << "testtags" << "favorites" << "1" << TagsStorage::TagErrorThingNotFound;
}

void TestTags::addTag()
{
    QFETCH(ThingId, thingId);
    QFETCH(QString, appId);
    QFETCH(QString, tagId);
    QFETCH(QString, value);
    QFETCH(TagsStorage::TagError, expectedError);

    enableNotifications({"Tags"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // Create a tag;
    QVariantMap params;
    params.insert("tag", createThingTag(thingId.toString(), appId, tagId, value));
    QVariant response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, expectedError);

    if (expectedError != TagsStorage::TagErrorNoError) {
    // If we expected an error, we can drop out here
        return;
    }

    // Make sure the TagAdded notification is emitted.
    QVariantMap notificationTagMap = checkNotification(clientSpy, "Tags.TagAdded").toMap().value("params").toMap().value("tag").toMap();
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareThingTag(notificationTagMap, thingId, appId, tagId, value), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());

    // Try getting the tag via GetTag
    params.clear();
    params.insert("thingId", thingId.toString());
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    QVariantList tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareThingTag(tagsList.first().toMap(), thingId, appId, tagId, value), "Fetched tag isn't matching the one we added");
}

void TestTags::updateTagValue()
{
    enableNotifications({"Tags"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    QString thingId = m_mockThingId.toString();
    QString appId = "testtags";
    QString tagId = "changedNotificationTag";

    // Create a Tag
    QVariantMap params;
    params.insert("tag", createThingTag(thingId, appId, tagId, "1"));
    QVariant response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagAdded notification
    QVariantMap notificationTagMap = checkNotification(clientSpy, "Tags.TagAdded").toMap().value("params").toMap().value("tag").toMap();
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareThingTag(notificationTagMap, QUuid(thingId), appId, tagId, "1"), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());
    clientSpy.clear();

    // Try getting the changed tag via GetTag
    params.clear();
    params.insert("thingId", thingId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    QVariantList tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareThingTag(tagsList.first().toMap(), QUuid(thingId), appId, tagId, "1"), "Fetched tag isn't matching the one we added");

    // Now update the tag
    params.clear();
    params.insert("tag", createThingTag(thingId, appId, tagId, "2"));
    response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagAdded notification
    notificationTagMap = checkNotification(clientSpy, "Tags.TagValueChanged").toMap().value("params").toMap().value("tag").toMap();
    jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareThingTag(notificationTagMap, QUuid(thingId), appId, tagId, "2"), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());

    // Try getting the changed tag via GetTag
    params.clear();
    params.insert("thingId", thingId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareThingTag(tagsList.first().toMap(), QUuid(thingId), appId, tagId, "2"), "Fetched tag isn't matching the one we added");
}

void TestTags::removeTag()
{
    enableNotifications({"Tags"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    QString thingId = m_mockThingId.toString();
    QString appId = "testtags";
    QString tagId = "removeTagTest";
    QString value = "1";

    // Create a Tag
    QVariantMap params;
    params.insert("tag", createThingTag(thingId, appId, tagId, value));
    QVariant response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagAdded notification
    QVariantMap notificationTagMap = checkNotification(clientSpy, "Tags.TagAdded").toMap().value("params").toMap().value("tag").toMap();
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareThingTag(notificationTagMap, QUuid(thingId), appId, tagId, value), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());
    clientSpy.clear();

    // Try getting the tag via GetTag
    params.clear();
    params.insert("thingId", thingId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    QVariantList tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 1);
    QVERIFY2(compareThingTag(tagsList.first().toMap(), QUuid(thingId), appId, tagId, value), "Fetched tag isn't matching the one we added");

    // Now remove the tag
    params.clear();
    params.insert("tag", createThingTag(thingId, appId, tagId, QString()));
    response = injectAndWait("Tags.RemoveTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Check for TagRemoved notification
    notificationTagMap = checkNotification(clientSpy, "Tags.TagRemoved").toMap().value("params").toMap().value("tag").toMap();
    jsonDoc = QJsonDocument::fromVariant(notificationTagMap);
    QVERIFY2(compareThingTag(notificationTagMap, QUuid(thingId), appId, tagId, QString()), QString("Tag in notification not matching: %1").arg(qUtf8Printable(jsonDoc.toJson())).toLatin1());

    // Try getting the tag via GetTag
    params.clear();
    params.insert("thingId", thingId);
    params.insert("appId", appId);
    params.insert("tagId", tagId);
    response = injectAndWait("Tags.GetTags", params);
    tagsList = response.toMap().value("params").toMap().value("tags").toList();
    QCOMPARE(tagsList.count(), 0);
}

void TestTags::ruleTagIsRemovedOnRuleRemove()
{
    // Create a rule
    QVariantMap params;
    params.insert("name", "testrule");
    QVariantMap action;
    action.insert("thingId", m_mockThingId);
    action.insert("actionTypeId", mockWithoutParamsActionTypeId);
    QVariantList actions = {action};
    params.insert("actions", actions);
    QVariant response = injectAndWait("Rules.AddRule", params);
    verifyError(response, "ruleError", "RuleErrorNoError");
    QUuid ruleId = response.toMap().value("params").toMap().value("ruleId").toUuid();

    // Tag the rule
    params.clear();
    QVariantMap tag;
    tag.insert("appId", "testtags");
    tag.insert("ruleId", ruleId);
    tag.insert("tagId", "testtag");
    tag.insert("value", "blabla");
    params.insert("tag", tag);
    response = injectAndWait("Tags.AddTag", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);

    // Make sure the tag is here
    params.clear();
    params.insert("appId", "testtags");
    params.insert("ruleId", ruleId);
    params.insert("tagId", "testtag");
    response = injectAndWait("Tags.GetTags", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);
    QVERIFY2(response.toMap().value("params").toMap().value("tags").toList().count() == 1, "Tag not found!");
    qCDebug(dcTests()) << "Get tag reply" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());

    // Remove the rule
    params.clear();
    params.insert("ruleId", ruleId);
    response = injectAndWait("Rules.RemoveRule", params);
    verifyError(response, "ruleError", "RuleErrorNoError");

    // Make sure the tag disappeared
    params.clear();
    params.insert("appId", "testtags");
    params.insert("ruleId", ruleId);
    params.insert("tagId", "testtag");
    response = injectAndWait("Tags.GetTags", params);
    verifyTagError(response, TagsStorage::TagErrorNoError);
    QVERIFY2(response.toMap().value("params").toMap().value("tags").toList().count() == 0, "Tag has not been cleaned up!");
    qCDebug(dcTests()) << "Get tag reply" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
}

#include "testtags.moc"
QTEST_MAIN(TestTags)
