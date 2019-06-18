/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "tokeninfo.h"

#include <QObject>
#include <QSqlDatabase>

namespace nymeaserver {

class PushButtonDBusService;

class UserManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(UserError)
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

    explicit UserManager(const QString &dbName, QObject *parent = nullptr);

    bool initRequired() const;
    QStringList users() const;

    UserError createUser(const QString &username, const QString &password);
    UserError removeUser(const QString &username);

    bool pushButtonAuthAvailable() const;

    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName);
    int requestPushButtonAuth(const QString &deviceName);
    void cancelPushButtonAuth(int transactionId);
    QString userForToken(const QByteArray &token) const;
    QList<TokenInfo> tokens(const QString &username) const;
    nymeaserver::UserManager::UserError removeToken(const QUuid &tokenId);

    bool verifyToken(const QByteArray &token);

signals:
    void pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

private:
    bool initDB();
    void rotate(const QString &dbName);
    bool validateUsername(const QString &username) const;
    bool validatePassword(const QString &password) const;
    bool validateToken(const QByteArray &token) const;

private slots:
    void onPushButtonPressed();

private:
    QSqlDatabase m_db;
    PushButtonDBusService *m_pushButtonDBusService = nullptr;
    int m_pushButtonTransactionIdCounter = 0;
    QPair<int, QString> m_pushButtonTransaction;

};
}
Q_DECLARE_METATYPE(nymeaserver::UserManager::UserError)

#endif // USERMANAGER_H
