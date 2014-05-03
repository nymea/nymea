/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>

class TestVersioning: public GuhTestBase
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
    qDebug() << "Got version:" << version << "( Expected:" << GUH_VERSION_STRING << ")";

    QVERIFY2(!version.isEmpty(), "Version is empty.");
    QCOMPARE(version, QString(GUH_VERSION_STRING));
}

void TestVersioning::apiChangeBumpsVersion()
{
    QString oldFilePath = QString(TESTS_SOURCE_DIR) + "/api.json";
    QString newFilePath = QString(TESTS_SOURCE_DIR) + "/api.json.new";

    QVariant response = injectAndWait("JSONRPC.Version", QVariantMap());
    QByteArray newVersion = response.toMap().value("params").toMap().value("version").toByteArray();

    response = injectAndWait("JSONRPC.Introspect", QVariantMap());
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(response.toMap().value("params"));
    QByteArray newApi = jsonDoc.toJson();


    QFile oldApiFile(oldFilePath);
    QVERIFY(oldApiFile.exists() && oldApiFile.open(QIODevice::ReadOnly));

    QByteArray oldVersion = oldApiFile.readLine().trimmed();
    QByteArray oldApi = oldApiFile.readAll();

    qDebug() << "version" << oldVersion << newVersion;

    if (oldVersion == newVersion && oldApi == newApi) {
        // All fine. no changes
        return;
    }

    if (oldVersion == newVersion && oldApi != newApi) {
        QVERIFY2(false, "JSONRPC API has changed but version is still the same. You need to bump the API version.");
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
    p.execute("diff", QStringList() << "-u" << oldFilePath << newFilePath);
    p.waitForFinished();
    qDebug() << p.readAll();

    QVERIFY2(false, QString("JSONRPC API has changed. Update %1.").arg(oldFilePath).toLatin1().data());
}

#include "testversioning.moc"
QTEST_MAIN(TestVersioning)
