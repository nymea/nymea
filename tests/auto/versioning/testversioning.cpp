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
#include "version.h"

#include <QRegularExpression>

using namespace nymeaserver;

class TestVersioning: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();

    void version();
    void apiChangeBumpsVersion();
};

void TestVersioning::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\nJsonRpcTraffic.debug=false\nJsonRpc.debug=true\nTests.debug=true");
    qCDebug(dcTests()) << "TestVersioning starting";
}


void TestVersioning::version()
{
    QVariant response = injectAndWait("JSONRPC.Version");

    QString version = response.toMap().value("params").toMap().value("version").toString();
    QVariant protocolVersion = response.toMap().value("params").toMap().value("protocol version");
    qDebug() << "Got version:" << version << "( Expected:" << NYMEA_VERSION_STRING << ")";

    QVERIFY2(!version.isEmpty(), "Version is empty.");
    QCOMPARE(version, QString(NYMEA_VERSION_STRING));

    QVERIFY2(!protocolVersion.toString().isEmpty(), "Protocol version is empty.");
    QVERIFY2(protocolVersion.canConvert(QMetaType::Int), "Protocol version is not an integer.");
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
    newVersionStripped = newVersionStripped.remove(QRegularExpression("\\+[0-9\\.~a-f]*"));

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

    qCDebug(dcTests()) << "API Differences:" << '\n' << qUtf8Printable(apiDiff);

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
