/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "guhsettings.h"
#include "logging/logentry.h"
#include "plugin/deviceplugin.h"

#include <QDebug>
#include "guhsettings.h"

using namespace guhserver;

class TestSettings : public GuhTestBase
{
    Q_OBJECT

private:


private slots:
    void getSetLanguages();


};

void TestSettings::getSetLanguages()
{
    enableNotifications();

    QVariant response; QVariantMap params;
    response = injectAndWait("Configuration.GetAvailableLanguages");
    QVERIFY2(response.toMap().value("params").toMap().contains("languages"), "Did not get list of languages");
    QVariantMap responseMap = response.toMap().value("params").toMap();
    QVERIFY2(responseMap.value("languages").toList().count() >= 2, "Avaliable languages list to short: " +  responseMap.value("languages").toList().count());

    QVariantList languageVariantList = responseMap.value("languages").toList();


    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    foreach (const QVariant &languageVariant, languageVariantList) {

        // Get current configurations
        response = injectAndWait("Configuration.GetConfigurations");
        QVariantMap configurationMap = response.toMap().value("params").toMap();

        params.clear();
        params.insert("language", languageVariant);

        notificationSpy.clear();

        // Set language
        QVariant response = injectAndWait("Configuration.SetLanguage", params);
        verifyConfigurationError(response);

        // Check notification
        notificationSpy.wait(500);
        QVariantList configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.LanguageChanged");
        printJson(configurationChangedNotifications);

        // If the language did not change no notification should be emited
        if (configurationMap.value("basicConfiguration").toMap().value("language").toString() == languageVariant.toString()) {
            QVERIFY2(configurationChangedNotifications.count() == 0, "Got Configuration.LanguageChanged notification but should have not.");
        } else {
            QVERIFY2(configurationChangedNotifications.count() == 1, "Should get only one Configuration.LanguageChanged notification");
        }
    }

    // Reset the language to en_US
    params.clear(); response.clear();
    params.insert("language", "en_US");

    // Set language
    response = injectAndWait("Configuration.SetLanguage", params);
    verifyConfigurationError(response);
}

#include "testsettings.moc"
QTEST_MAIN(TestSettings)

