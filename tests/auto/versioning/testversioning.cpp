/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

class TestVersioning: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void version();
    void apiChangeBumpsVersion();
};

void TestVersioning::version()
{
    QVariant response = injectAndWait("JSONRPC.Version");

    QString version = response.toMap().value("params").toMap().value("version").toString();
    QVariant protocolVersion = response.toMap().value("params").toMap().value("protocol version");
    qDebug() << "Got version:" << version << "( Expected:" << NYMEA_VERSION_STRING << ")";

    QVERIFY2(!version.isEmpty(), "Version is empty.");
    QCOMPARE(version, QString(NYMEA_VERSION_STRING));

    QVERIFY2(!protocolVersion.toString().isEmpty(), "Protocol version is empty.");
    QVERIFY2(protocolVersion.canConvert(QVariant::Int), "Protocol version is not an integer.");
}

void TestVersioning::apiChangeBumpsVersion()
{
    QString oldFilePath = QString(TESTS_SOURCE_DIR) + "/api.json";
    QString newFilePath = QString(TESTS_SOURCE_DIR) + "/api.json.new";

    QVariant response = injectAndWait("JSONRPC.Version", QVariantMap());
    QByteArray newVersion = response.toMap().value("params").toMap().value("protocol version").toByteArray();

    response = injectAndWait("JSONRPC.Introspect", QVariantMap());
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(response.toMap().value("params"));
    QByteArray newApi = jsonDoc.toJson();


    QFile oldApiFile(oldFilePath);
    QVERIFY(oldApiFile.exists() && oldApiFile.open(QIODevice::ReadOnly));

    QByteArray oldVersion = oldApiFile.readLine().trimmed();
    QByteArray oldApi = oldApiFile.readAll();

    QString newVersionStripped = newVersion;
    newVersionStripped = newVersionStripped.remove(QRegExp("\\+[0-9\\.~a-f]*"));

    qDebug() << "JSON API version:" << oldVersion;
    qDebug() << "Binary version:" << newVersion << "(" + newVersionStripped + ")";

    if (oldVersion == newVersionStripped && oldApi == newApi) {
        // All fine. no changes
        return;
    }

    QFile newApiFile(newFilePath);
    QVERIFY(newApiFile.open(QIODevice::ReadWrite));
    if (newApiFile.size() > 0) {
        newApiFile.resize(0);
    }

    newApiFile.write(newVersion + '\n');
    newApiFile.write(newApi);
    newApiFile.flush();

    QProcess p;
    p.start("diff", QStringList() << "-u" << oldFilePath << newFilePath);
    p.waitForFinished();
    QByteArray apiDiff = p.readAll();

    qDebug() << "API Differences:" << endl << qUtf8Printable(apiDiff);

    if (oldVersion == newVersionStripped && oldApi != newApi) {
        QVERIFY2(false, "JSONRPC API has changed but version is still the same. You need to bump the API version.");
    }

    if (oldVersion != newVersionStripped && oldApi == newApi) {
        QVERIFY2(false, QString("Version has changed. Update %1.").arg(oldFilePath).toLatin1().data());
    } else {
        QVERIFY2(false, QString("JSONRPC API has changed. Update %1.").arg(oldFilePath).toLatin1().data());
    }
}

#include "testversioning.moc"
QTEST_MAIN(TestVersioning)
