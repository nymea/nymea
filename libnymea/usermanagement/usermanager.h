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

#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>

#include "tokeninfo.h"
#include "userinfo.h"

class CreateUserInfo;

class UserManager : public QObject
{
    Q_OBJECT
public:
    enum UserError {
        UserErrorNoError,
        UserErrorBackendError,
        UserErrorInvalidUserId,
        UserErrorDuplicateUserId,
        UserErrorBadPassword,
        UserErrorTokenNotFound,
        UserErrorPermissionDenied
    };
    Q_ENUM(UserError)

    enum Capability {
        CapabilityNone = 0x00,
        CapabilityPushButton = 0x01,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)


    explicit UserManager(QObject *parent = nullptr);
    virtual ~UserManager() = default;

    virtual bool isReady() const = 0;
    virtual Capabilities capabilities() const = 0;
    virtual bool initRequired() const = 0;

    virtual UserInfoList users() const = 0;
    virtual CreateUserInfo* createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes) = 0;
    virtual UserError removeUser(const QString &username) = 0;
    virtual UserInfo userInfo(const QString &username = QString()) const = 0;
    virtual UserError setUserInfo(const QString &username, const QString &email, const QString &displayName) = 0;
    virtual UserError changePassword(const QString &username, const QString &newPassword) = 0;
    virtual UserError setUserScopes(const QString &username, Types::PermissionScopes scopes) = 0;

    virtual QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName) = 0;
    virtual int requestPushButtonAuth(const QString &deviceName) = 0;
    virtual void cancelPushButtonAuth(int transactionId) = 0;

    virtual bool verifyToken(const QByteArray &token) = 0;
    virtual QList<TokenInfo> tokens(const QString &username) const = 0;
    virtual TokenInfo tokenInfo(const QByteArray &token) const = 0;
    virtual TokenInfo tokenInfo(const QUuid &tokenId) const = 0;
    virtual UserError removeToken(const QUuid &tokenId) = 0;

signals:
    void readyChanged(bool ready);
    void userAdded(const QString &username);
    void userRemoved(const QString &username);
    void userChanged(const QString &username);
    void pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

};
Q_DECLARE_METATYPE(UserManager::UserError)

#endif // USERMANAGER_H
