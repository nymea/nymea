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

using namespace nymeaserver;

class TestDebugHandler: public NymeaTestBase
{
    Q_OBJECT

protected slots:
    void initTestCase();

private slots:
    void getLoggingFilters();

};

void TestDebugHandler::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\nApplication.debug=true\nTests.debug=true\nServerManager.debug=true");
}

void TestDebugHandler::getLoggingFilters()
{
    QVariant response = injectAndWait("Debug.GetLoggingCategories");
    QVariantMap loggingFilters = response.toMap().value("params").toMap();

    QVERIFY(loggingFilters.contains("loggingCategories"));
    QVariantList loggingCategoriesList = loggingFilters.value("loggingCategories").toList();
    QVERIFY(!loggingCategoriesList.isEmpty());

    foreach(const QVariant &categoryVariant, loggingCategoriesList) {
        QVariantMap categoryMap = categoryVariant.toMap();
        QVERIFY(categoryMap.contains("level"));
        QVERIFY(categoryMap.contains("name"));
        QVERIFY(categoryMap.contains("type"));
    }
}

#include "testdebughandler.moc"
QTEST_MAIN(TestDebugHandler)
