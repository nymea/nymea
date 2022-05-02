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

#ifndef USERMANAGERIMPLEMENTATION_H
#define USERMANAGERIMPLEMENTATION_H

#include "usermanagement/usermanager.h"

#include <QObject>

class UserBackend;

namespace nymeaserver {

class UserManagerImplementation : public UserManager
{
    Q_OBJECT
public:

    explicit UserManagerImplementation(QObject *parent = nullptr);

    Capabilities capabilities() const override;

    bool isReady() const override;
    bool initRequired() const override;
    UserInfoList users() const override;

    CreateUserInfo* createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes) override;
    UserError changePassword(const QString &username, const QString &newPassword) override;
    UserError removeUser(const QString &username) override;
    UserError setUserScopes(const QString &username, Types::PermissionScopes scopes) override;
    UserError setUserInfo(const QString &username, const QString &email, const QString &displayName) override;


    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName) override;
    int requestPushButtonAuth(const QString &deviceName) override;
    void cancelPushButtonAuth(int transactionId) override;

    UserInfo userInfo(const QString &username = QString()) const override;
    TokenInfo tokenInfo(const QByteArray &token) const override;
    TokenInfo tokenInfo(const QUuid &tokenId) const override;
    QList<TokenInfo> tokens(const QString &username) const override;

    UserError removeToken(const QUuid &tokenId) override;


    bool verifyToken(const QByteArray &token) override;

private:
    QStringList pluginSearchDirs();
    void loadBackendPlugin(const QString &file);

private:
    UserBackend *m_backend = nullptr;
};
}

#endif // USERMANAGERIMPLEMENTATION_H
