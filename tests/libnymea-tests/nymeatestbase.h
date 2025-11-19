// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NYMEATESTBASE_H
#define NYMEATESTBASE_H

#include <QSignalSpy>
#include <QtTest>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLoggingCategory>

#include <typeutils.h>

Q_DECLARE_LOGGING_CATEGORY(dcTests)

namespace nymeaserver {
class MockTcpServer;

class NymeaTestBase : public QObject
{
    Q_OBJECT
public:
    explicit NymeaTestBase(QObject *parent = nullptr);

protected slots:
    void initTestCase(const QString &loggingRules = QString(), bool disableLogEngine = false);
    void cleanupTestCase();
    void cleanup();

protected:
    QVariant injectAndWait(const QString &method, const QVariantMap &params = QVariantMap(), const QUuid &clientId = QUuid(), const QByteArray &tokenOverride = "default");
    QVariant checkNotification(const QSignalSpy &spy, const QString &notification);
    QVariantList checkNotifications(const QSignalSpy &spy, const QString &notification);

    QVariant getAndWait(const QNetworkRequest &request, const int &expectedStatus = 200);
    QVariant deleteAndWait(const QNetworkRequest &request, const int &expectedStatus = 200);
    QVariant postAndWait(const QNetworkRequest &request, const QVariant &params, const int &expectedStatus = 200);
    QVariant putAndWait(const QNetworkRequest &request, const QVariant &params, const int &expectedStatus = 200);

    void verifyReply(QNetworkReply *reply, const QByteArray &data, const int &expectedStatus);

    void enableNotifications(const QStringList &namespaces);
    bool disableNotifications();

    inline void verifyError(const QVariant &response, const QString &fieldName, const QString &error)
    {
        QJsonDocument jsonDoc = QJsonDocument::fromVariant(response);
        QVERIFY2(response.toMap().value("status").toString() == QString("success"),
                 QString("\nExpected status: \"success\"\nGot: %2\nFull message: %3")
                     .arg(response.toMap().value("status").toString())
                     .arg(jsonDoc.toJson().data())
                     .toLatin1().data());
        QVERIFY2(response.toMap().value("params").toMap().value(fieldName).toString() == error,
                 QString("\nExpected: %1\nGot: %2\nFull message: %3\n")
                     .arg(error)
                     .arg(response.toMap().value("params").toMap().value(fieldName).toString())
                     .arg(jsonDoc.toJson().data())
                     .toLatin1().data());
    }

    template<typename T> QString enumValueName(T value)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        return metaEnum.valueToKey(value);
    }

    template<typename T> T enumNameToValue(const QString &name) {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        return static_cast<T>(metaEnum.keyToValue(name.toUtf8()));
    }


    inline void verifyParams(const QVariantList &requestList, const QVariantList &responseList, bool allRequired = true)
    {
        if (allRequired)
            QVERIFY2(requestList.count() == responseList.count(), "Not the same count of param in response.");

        foreach (const QVariant &requestParam, requestList) {
            bool found = false;
            foreach (const QVariant &responseParam, responseList) {
                if (requestParam.toMap().value("paramTypeId").toUuid() == responseParam.toMap().value("paramTypeId").toUuid()){
                    QCOMPARE(requestParam.toMap().value("value"), responseParam.toMap().value("value"));
                    found = true;
                    break;
                }
            }
            if (allRequired)
                QVERIFY2(found, "Param missing");
        }
    }

    // just for debugging
    inline void printJson(const QVariant &response) {
        QJsonDocument jsonDoc = QJsonDocument::fromVariant(response);
        qCDebug(dcTests()) << jsonDoc.toJson();
    }

    void waitForDBSync();
    void restartServer();
    void clearLoggingDatabase(const QString &source);

private:
    void createMock();

protected:
    bool m_disableLogEngine = false;

    PluginId mockPluginId = PluginId("727a4a9a-c187-446f-aadf-f1b2220607d1");
    VendorId nymeaVendorId = VendorId("2062d64d-3232-433c-88bc-0d33c0ba2ba6");

    MockTcpServer *m_mockTcpServer;
    QUuid m_clientId;
    int m_commandId;

    int m_mockThing1Port;
    int m_mockThing2Port;

    ThingId m_mockThingId;
    ThingId m_mockThingAutoId;
    QByteArray m_apiToken;

};

}

#endif // NYMEATESTBASE_H
