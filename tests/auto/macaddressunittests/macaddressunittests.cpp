/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#include <network/macaddress.h>

using namespace nymeaserver;

class MacAddressUnitTests: public NymeaTestBase
{
    Q_OBJECT

protected slots:
    void initTestCase();

private:
    QString m_zeroMacString = QString("00:00:00:00:00:00");
    QByteArray m_zeroMacByteArray = QByteArray(6, '\0');

    QString m_alphaNumericMacString = QString("01:23:45:ab:cd:ef");
    QByteArray m_alphaNumericByteArray = QByteArray::fromHex("012345abcdef");

private slots:
    void defaultConstructor();

    void macAddressValidation_data();
    void macAddressValidation();

};

void MacAddressUnitTests::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\nTests.debug=true\n");
}

void MacAddressUnitTests::defaultConstructor()
{
    MacAddress mac;
    QVERIFY(mac.isNull());
    QVERIFY(mac.isValid());
    QCOMPARE(mac.toByteArray().count(), 6);
    QCOMPARE(mac.toByteArray(), QByteArray(6, '\0'));
    QVERIFY(mac.toString() == m_zeroMacString);

    QVERIFY(!MacAddress(QString("acme")).isValid());
    QVERIFY(MacAddress(QString("acme")).isNull());
    QVERIFY(!MacAddress(QByteArray()).isValid());
    QVERIFY(MacAddress(QByteArray()).isNull());

    QVERIFY(MacAddress(QByteArray()).toByteArray().isEmpty());

    QCOMPARE(MacAddress(m_zeroMacByteArray).toString(), m_zeroMacString);
    QCOMPARE(MacAddress(m_zeroMacString).toByteArray(), m_zeroMacByteArray);

    QCOMPARE(MacAddress(m_alphaNumericByteArray).toString(), m_alphaNumericMacString);
    QCOMPARE(MacAddress(m_alphaNumericMacString).toByteArray(), m_alphaNumericByteArray);

    QByteArray validRawData = QByteArray::fromHex("aabbccddeeff");
    QCOMPARE(MacAddress(QString("aa:bb:cc:dd:ee:ff")).toByteArray(), validRawData);
    QCOMPARE(MacAddress(QString("aa:bb:cc:dd:ee:ff")), MacAddress(validRawData));
    QCOMPARE(MacAddress(QString("aabbccddeeff")).toByteArray(), validRawData);
    QCOMPARE(MacAddress(QString("aabbccddeeff")), MacAddress(validRawData));
}

void MacAddressUnitTests::macAddressValidation_data()
{
    QTest::addColumn<QString>("macString");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<bool>("isNull");
    QTest::addColumn<QString>("toString");

    QString mixedString = "11:22:33:dd:ee:ff";
    QString aplhaString = "aa:bb:cc:dd:ee:ff";

    QTest::newRow("Valid zero") << "00:00:00:00:00:00" << true << true << m_zeroMacString;
    QTest::newRow("Valid zero no colon") << "000000000000" << true << true << m_zeroMacString;
    QTest::newRow("Valid non zero lower") << "11:22:33:dd:ee:ff" << true << false << mixedString;
    QTest::newRow("Valid non zero upper") << "11:22:33:DD:EE:FF" << true << false << mixedString;
    QTest::newRow("Valid non zero mixed case") << "11:22:33:Dd:Ee:Ff" << true << false << mixedString;
    QTest::newRow("Valid non zero space separator") << "aa bb cc dd ee ff" << true << false << aplhaString;
    QTest::newRow("Valid non zero dash separator") << "aa-bb-cc-dd-ee-ff" << true << false << aplhaString;
    QTest::newRow("Valid non zero dot separator") << "aa.bb.cc.dd.ee.ff" << true << false << aplhaString;
    QTest::newRow("Valid non zero crazy separator") << "aa#bb?cclddxeeäff" << true << false << aplhaString;
    QTest::newRow("Valid non zero mixed separators") << "aa-bb cc.dd ee-ff" << true << false << aplhaString;
    QTest::newRow("Valid non zero without colon") << "aabbccddeeff" << true << false << aplhaString;
    QTest::newRow("Invalid characters") << "xx:yy:zz:dd:ee:ff" << false << true << m_zeroMacString;
    QTest::newRow("Too short") << "xx:yy:zz:dd:ee" << false << true << m_zeroMacString;
    QTest::newRow("Too long") << "xx:yy:zz:dd:ee:ee:ee" << false << true << m_zeroMacString;
}

void MacAddressUnitTests::macAddressValidation()
{
    QFETCH(QString, macString);
    QFETCH(bool, isValid);
    QFETCH(bool, isNull);
    QFETCH(QString, toString);

    MacAddress mac(macString);
    qCDebug(dcTests()) << "Verify" << macString << "resulting in" << mac;
    QCOMPARE(mac.isValid(), isValid);
    QCOMPARE(mac.isNull(), isNull);
    QCOMPARE(mac.toString(), toString);
}

#include "macaddressunittests.moc"
QTEST_MAIN(MacAddressUnitTests)
