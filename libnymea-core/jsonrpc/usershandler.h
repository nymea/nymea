#ifndef USERSHANDLER_H
#define USERSHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"

namespace nymeaserver {

class UserManager;

class UsersHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit UsersHandler(UserManager *userManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *CreateUser(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ChangePassword(const QVariantMap &params);
    Q_INVOKABLE JsonReply *Authenticate(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RequestPushButtonAuth(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetUserInfo(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetTokens(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveToken(const QVariantMap &params);

signals:
    void PushButtonAuthFinished(const QVariantMap &params);

private:
    UserManager *m_userManager = nullptr;
    QHash<int, QUuid> m_pushButtonTransactions;

};

}

#endif // USERSHANDLER_H
