/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2025, nymea GmbH
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

#ifndef TESTUSERMANAGER_H
#define TESTUSERMANAGER_H

#include <QtTest>
#include "nymeatestbase.h"

using namespace nymeaserver;

class TestUsermanager: public NymeaTestBase
{
    Q_OBJECT
public:
    TestUsermanager(QObject* parent = nullptr);

private slots:
    void initTestCase();

    void init();

    void loginValidation_data();
    void loginValidation();

    void createUser();

    void authenticate();

    /*
    Cases for push button auth:

    Case 1: regular pushbutton
    - alice sends Users.RequestPushButtonAuth, gets "OK" back (if push button hardware is available)
    - alice pushes the hardware button and gets a notification on jsonrpc containing the token for local auth
    */
    void authenticatePushButton();

    /*
    Case 2: if we have an attacker in the network, he could try to call requestPushButtonAuth and
    hope someone would eventually press the button and give him a token. In order to prevent this,
    any previous attempt for a push button auth needs to be cancelled when a new request comes in:

    * Mallory does RequestPushButtonAuth, gets OK back
    * Alice does RequestPushButtonAuth,
    * Mallory receives a "PushButtonFailed" notification
    * Alice receives OK
    * Alice presses the hardware button
    * Alice reveices a notification with token, mallory receives nothing

    Case 3: Mallory tries to hijack it back again

    * Mallory does RequestPushButtonAuth, gets OK back
    * Alice does RequestPusButtonAuth,
    * Alice gets ok reply, Mallory gets failed notification
    * Mallory quickly does RequestPushButtonAuth again to win the fight
    * Alice gets failed notification and can instruct the user to _not_ press the button now until procedure is restarted
    */
    void authenticatePushButtonAuthInterrupt();

    void authenticatePushButtonAuthConnectionDrop();

    void createDuplicateUser();

    void getTokens();

    void removeToken();

    void unauthenticatedCallAfterTokenRemove();

    void changePassword();

    void authenticateAfterPasswordChangeOK();

    void authenticateAfterPasswordChangeFail();

    void getUserInfo();

    void testScopeConsitancy_data();
    void testScopeConsitancy();

    void testRestrictedThingAccess();

private:
    // m_apiToken is in testBase
    QUuid m_tokenId;

    void authenticateTestuser(const QString &username);

    QString m_usernameAdmin = "admin";
    QString m_usernameGuest = "guest";

    QByteArray m_adminToken;
    QByteArray m_guestToken;

};

#endif // TESTUSERMANAGER_H
