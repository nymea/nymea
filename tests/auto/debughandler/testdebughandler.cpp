/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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
