// SPDX-License-Identifier: LGPL-3.0-or-later

#include "nymeatestbase.h"
#include "nymeaconfiguration.h"
#include "nymeacore.h"
#include "nymeasettings.h"

#include <QFile>
#include <QPointer>
#include <QTemporaryDir>
#include <QtTest>

using namespace nymeaserver;

class TestFactoryReset : public NymeaTestBase
{
    Q_OBJECT

private slots:
    // Reset removes the test user; these tests call core directly afterwards.
    void cleanup() {}

    void internalReset_data()
    {
        QTest::addColumn<bool>("emptySetting");
        QTest::newRow("absent") << false;
        QTest::newRow("empty") << true;
    }

    void internalReset()
    {
        QFETCH(bool, emptySetting);
        {
            NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
            settings.remove("FactoryReset/script");
            if (emptySetting)
                settings.setValue("FactoryReset/script", "");
            settings.sync();
        }
        QCOMPARE(NymeaCore::instance()->configuration()->factoryResetScript(), QString());
        const QString marker = NymeaSettings::settingsPath() + "/factory-reset-marker";
        QFile file(marker);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        const QString cacheMarker = NymeaSettings::cachePath() + "/factory-reset-marker";
        QVERIFY(QDir().mkpath(NymeaSettings::cachePath()));
        QFile cacheFile(cacheMarker);
        QVERIFY(cacheFile.open(QIODevice::WriteOnly));
        cacheFile.close();
        QPointer<NymeaCore> oldCore = NymeaCore::instance();
        QVERIFY(oldCore->requestFactoryReset());
        QVERIFY(oldCore); // Teardown remains queued.
        // Factory reset deletes the dummy user and its token. Hello supports an
        // anonymous handshake; retaining the old token triggers client lockdown.
        m_apiToken.clear();
        waitForServerRestart();
        QVERIFY(oldCore.isNull());
        QTRY_VERIFY(!QFile::exists(marker));
        QVERIFY(!QFile::exists(cacheMarker));
    }

    void externalReset()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString script = directory.path() + "/reset script";
        const QString marker = NymeaSettings::settingsPath() + "/factory-reset-marker";
        QFile markerFile(marker);
        QVERIFY(markerFile.open(QIODevice::WriteOnly));
        markerFile.close();
        const QString cacheMarker = NymeaSettings::cachePath() + "/external-reset-marker";
        QVERIFY(QDir().mkpath(NymeaSettings::cachePath()));
        QFile cacheFile(cacheMarker);
        QVERIFY(cacheFile.open(QIODevice::WriteOnly));
        cacheFile.close();
        QPointer<NymeaCore> core = NymeaCore::instance();
        QSignalSpy destroyed(core.data(), &QObject::destroyed);

        auto configure = [](const QString &path) {
            NymeaSettings settings(NymeaSettings::SettingsRoleGlobal);
            settings.setValue("FactoryReset/script", path);
            settings.sync();
        };
        const QStringList invalidPaths = {"relative/script", script, directory.path()};
        foreach (const QString &path, invalidPaths) {
            configure(path);
            QVERIFY(!core->requestFactoryReset());
        }

        QFile scriptFile(script);
        QVERIFY(scriptFile.open(QIODevice::WriteOnly));
        scriptFile.write("#!/missing-factory-reset-interpreter\n");
        scriptFile.close();
        QVERIFY(scriptFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner));
        configure(script);
        QVERIFY(!core->requestFactoryReset()); // Non-executable.
        QVERIFY(scriptFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        QVERIFY(!core->requestFactoryReset()); // Valid file, but exec fails.
        QVERIFY(!core->requestFactoryReset()); // Launch failure permits retry.
        QTest::qWait(100);
        QVERIFY(core);
        QVERIFY(QFile::exists(marker));
        QVERIFY(QFile::exists(cacheMarker));
        QCOMPARE(destroyed.count(), 0);

        // Replace the same file with a working script. Record cwd and argument count.
        QVERIFY(scriptFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        scriptFile.write("#!/bin/sh\nprintf '%s\\n' \"$PWD\" \"$#\" > \"$0.result\"\n");
        scriptFile.close();
        QVERIFY(core->requestFactoryReset());
        QVERIFY(!core->requestFactoryReset());
        QTRY_VERIFY(QFile::exists(script + ".result"));
        QFile result(script + ".result");
        QVERIFY(result.open(QIODevice::ReadOnly));
        QTRY_COMPARE(result.size(), qint64(4));
        QCOMPARE(result.readAll(), QByteArray("/\n0\n"));
        QVERIFY(core);
        QVERIFY(QFile::exists(marker));
        QVERIFY(QFile::exists(cacheMarker));
        QCOMPARE(destroyed.count(), 0);
        configure("");
        QVERIFY(!core->requestFactoryReset()); // Cannot bypass guard by removing override.
        QVERIFY(QFile::exists(marker));
        restartServer();
        QVERIFY(!NymeaCore::instance()->requestFactoryReset());
        QVERIFY(QFile::exists(marker));
        QVERIFY(QFile::exists(cacheMarker));
    }
};

#include "testfactoryreset.moc"
QTEST_MAIN(TestFactoryReset)
