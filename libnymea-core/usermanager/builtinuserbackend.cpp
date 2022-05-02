#include "builtinuserbackend.h"
#include "nymeasettings.h"
#include "pushbuttondbusservice.h"
#include "usermanagement/createuserinfo.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QCryptographicHash>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcUserManager)

namespace nymeaserver {

BuiltinUserBackend::BuiltinUserBackend(QObject *parent)
    : UserBackend{parent}
{
    QString dbName = NymeaSettings::settingsPath() + "/user-db.sqlite";

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), "users");
    m_db.setDatabaseName(dbName);

    qCDebug(dcUserManager()) << "Opening user database" << m_db.databaseName();

    if (!m_db.isValid()) {
        qCWarning(dcUserManager()) << "The database is not valid:" << m_db.lastError().driverText() << m_db.lastError().databaseText();
        rotate(m_db.databaseName());
    }

    if (!initDB()) {
        qCWarning(dcUserManager()) << "Error initializing user database. Trying to correct it.";
        if (QFileInfo(m_db.databaseName()).exists()) {
            rotate(m_db.databaseName());
            if (!initDB()) {
                qCWarning(dcUserManager()) << "Error fixing user database. Giving up. Users can't be stored.";
            }
        }
    }

    m_pushButtonDBusService = new PushButtonDBusService("/io/guh/nymead/UserManager", this);
    connect(m_pushButtonDBusService, &PushButtonDBusService::pushButtonPressed, this, &BuiltinUserBackend::onPushButtonPressed);
    m_pushButtonTransaction = qMakePair<int, QString>(-1, QString());
}

UserManager::Capabilities BuiltinUserBackend::capabilities() const
{
    UserManager::Capabilities caps = UserManager::CapabilityNone;
    if (m_pushButtonDBusService->agentAvailable()) {
        caps |= UserManager::CapabilityPushButton;
    }
    return caps;
}

bool BuiltinUserBackend::initRequired() const
{
    QString getTokensQuery = QString("SELECT id, username, creationdate, deviceName FROM tokens;");
    QSqlQuery result = m_db.exec(getTokensQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for tokens failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokensQuery;
        // Note: do not return true in case the database access fails.
        return false;
    }

    return users().isEmpty() && !result.first();

}

UserInfoList BuiltinUserBackend::users() const
{
    QString userQuery("SELECT * FROM users;");
    QSqlQuery result = m_db.exec(userQuery);
    UserInfoList users;
    while (result.next()) {
        UserInfo info = UserInfo(result.value("username").toString());
        info.setEmail(result.value("email").toString());
        info.setDisplayName(result.value("displayName").toString());
        info.setScopes(Types::scopesFromStringList(result.value("scopes").toString().split(',')));
        users.append(info);
    }
    return users;
}

void BuiltinUserBackend::createUser(CreateUserInfo *info)
{
    if (!validateUsername(info->username())) {
        qCWarning(dcUserManager) << "Error creating user. Invalid username:" << info;
        info->finish(UserManager::UserErrorInvalidUserId);
        return;
    }

    if (!validatePassword(info->password())) {
        qCWarning(dcUserManager) << "Password failed character validation. Must contain a letter, a number and a special charactar. Minimum length: 8";
        info->finish(UserManager::UserErrorBadPassword);
        return;
    }

    QSqlQuery checkForDuplicateUserQuery(m_db);
    checkForDuplicateUserQuery.prepare("SELECT * FROM users WHERE lower(username) = ?;");
    checkForDuplicateUserQuery.addBindValue(info->username().toLower());
    checkForDuplicateUserQuery.exec();
    if (checkForDuplicateUserQuery.first()) {
        qCWarning(dcUserManager) << "Username already in use";
        info->finish(UserManager::UserErrorDuplicateUserId);
        return;
    }

    QByteArray salt = QUuid::createUuid().toString().remove(QRegExp("[{}]")).toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(QString(info->password() + salt).toUtf8(), QCryptographicHash::Sha512).toBase64();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users(username, email, displayName, password, salt, scopes) VALUES(?, ?, ?, ?, ?, ?);");
    query.addBindValue(info->username().toLower());
    query.addBindValue(info->email());
    query.addBindValue(info->displayName());
    query.addBindValue(QString::fromUtf8(hashedPassword));
    query.addBindValue(QString::fromUtf8(salt));
    query.addBindValue(Types::scopesToStringList(info->scopes()).join(','));
    query.exec();
    if (query.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error creating user:" << query.lastError().databaseText() << query.lastError().driverText();
        info->finish(UserManager::UserErrorBackendError);
        return;
    }

    qCInfo(dcUserManager()) << "New user" << info << "added to the system with permissions:" << Types::scopesToStringList(info->scopes());
    emit userAdded(info->username());
    info->finish(UserManager::UserErrorNoError);
}

UserManager::UserError BuiltinUserBackend::changePassword(const QString &username, const QString &newPassword)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Invalid username:" << username;
        return UserManager::UserErrorInvalidUserId;
    }

    if (!validatePassword(newPassword)) {
        qCWarning(dcUserManager) << "Password failed character validation. Must contain a letter, a number and a special charactar. Minimum length: 8";
        return UserManager::UserErrorBadPassword;
    }

    QString checkForUserExistingQuery = QString("SELECT * FROM users WHERE lower(username) = \"%1\";").arg(username.toLower());
    QSqlQuery result = m_db.exec(checkForUserExistingQuery);
    if (!result.first()) {
        qCWarning(dcUserManager) << "Username does not exist.";
        return UserManager::UserErrorInvalidUserId;
    }

    // Update the password
    QByteArray salt = QUuid::createUuid().toString().remove(QRegExp("[{}]")).toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(QString(newPassword + salt).toUtf8(), QCryptographicHash::Sha512).toBase64();
    QString queryString = QString("UPDATE users SET password = \"%1\", salt = \"%2\" WHERE lower(username) = \"%3\";")
            .arg(QString::fromUtf8(hashedPassword))
            .arg(QString::fromUtf8(salt))
            .arg(username.toLower());
    m_db.exec(queryString);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error updating password for user:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserManager::UserErrorBackendError;
    }
    qCDebug(dcUserManager()) << "Password updated for user" << username;
    return UserManager::UserErrorNoError;
}

UserManager::UserError BuiltinUserBackend::removeUser(const QString &username)
{
    QString dropUserQuery = QString("DELETE FROM users WHERE lower(username) =\"%1\";").arg(username.toLower());
    QSqlQuery result = m_db.exec(dropUserQuery);
    if (result.numRowsAffected() == 0) {
        return UserManager::UserErrorInvalidUserId;
    }

    QString dropTokensQuery = QString("DELETE FROM tokens WHERE lower(username) = \"%1\";").arg(username.toLower());
    m_db.exec(dropTokensQuery);

    emit userRemoved(username);
    return UserManager::UserErrorNoError;
}

UserManager::UserError BuiltinUserBackend::setUserScopes(const QString &username, Types::PermissionScopes scopes)
{
    QString scopesString = Types::scopesToStringList(scopes).join(',');
    QSqlQuery setScopesQuery(m_db);
    setScopesQuery.prepare("UPDATE users SET scopes = '%1' WHERE username = '%2'");
    setScopesQuery.addBindValue(scopesString);
    setScopesQuery.addBindValue(username);
    setScopesQuery.exec();
    if (setScopesQuery.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager()) << "Error updating scopes for user" << username << setScopesQuery.lastError().databaseText() << setScopesQuery.lastError().driverText();
        return UserManager::UserErrorBackendError;
    }

    emit userChanged(username);
    return UserManager::UserErrorNoError;
}

UserInfo BuiltinUserBackend::userInfo(const QString &username) const
{
    QString getUserQuery = QString("SELECT * FROM users WHERE lower(username) = \"%1\";")
            .arg(username);
    QSqlQuery result = m_db.exec(getUserQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for user" << username << "failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getUserQuery;
        return UserInfo();
    }

    if (!result.first()) {
        return UserInfo();
    }

    UserInfo userInfo = UserInfo(result.value("username").toString());
    userInfo.setEmail(result.value("email").toString());
    userInfo.setDisplayName(result.value("displayName").toString());
    userInfo.setScopes(Types::scopesFromStringList(result.value("scopes").toString().split(',')));

    return userInfo;
}

UserManager::UserError BuiltinUserBackend::setUserInfo(const QString &username, const QString &email, const QString &displayName)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET email = ?, displayName = ? WHERE username = ?;");
    query.addBindValue(email);
    query.addBindValue(displayName);
    query.addBindValue(username);
    query.exec();
    if (query.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager()) << "Error updating user info for user" << username << query.lastError().databaseText() << query.lastError().driverText() << query.executedQuery();
        return UserManager::UserErrorBackendError;
    }
    emit userChanged(username);
    return UserManager::UserErrorNoError;
}

QByteArray BuiltinUserBackend::authenticate(const QString &username, const QString &password, const QString &deviceName)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Authenticate: Username did not pass validation:" << username;
        return QByteArray();
    }

    QSqlQuery passwordQuery(m_db);
    passwordQuery.prepare("SELECT password, salt FROM users WHERE lower(username) = ?;");
    passwordQuery.addBindValue(username.toLower());
    passwordQuery.exec();
    if (!passwordQuery.first()) {
        qCWarning(dcUserManager) << "No such username" << username;
        return QByteArray();
    }
    QByteArray salt = passwordQuery.value("salt").toByteArray();
    QByteArray hashedPassword = passwordQuery.value("password").toByteArray();

    if (hashedPassword != QCryptographicHash::hash(QString(password + salt).toUtf8(), QCryptographicHash::Sha512).toBase64()) {
        qCWarning(dcUserManager) << "Authentication error for user:" << username;
        return QByteArray();
    }

    QByteArray token = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QString storeTokenQuery = QString("INSERT INTO tokens(id, username, token, creationdate, devicename) VALUES(\"%1\", \"%2\", \"%3\", \"%4\", \"%5\");")
            .arg(QUuid::createUuid().toString())
            .arg(username.toLower())
            .arg(QString::fromUtf8(token))
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(deviceName);

    m_db.exec(storeTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error storing token in DB:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return QByteArray();
    }
    return token;

}

QList<TokenInfo> BuiltinUserBackend::tokens(const QString &username) const
{
    QList<TokenInfo> ret;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, username, creationdate, deviceName FROM tokens WHERE lower(username) = ?;");
    query.addBindValue(username.toLower());
    query.exec();
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for tokens failed:" << query.lastError().databaseText() << query.lastError().driverText() << query.executedQuery();
        return ret;
    }

    while (query.next()) {
        ret << TokenInfo(query.value("id").toUuid(), query.value("username").toString(), query.value("creationdate").toDateTime(), query.value("devicename").toString());
    }
    return ret;
}

TokenInfo BuiltinUserBackend::tokenInfo(const QByteArray &token) const
{
    if (!validateToken(token)) {
        qCWarning(dcUserManager) << "Token did not pass validation:" << token;
        return TokenInfo();
    }

    QString getTokenQuery = QString("SELECT id, username, creationdate, deviceName FROM tokens WHERE token = \"%1\";")
            .arg(QString::fromUtf8(token));
    QSqlQuery result = m_db.exec(getTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokenQuery;
        return TokenInfo();
    }

    if (!result.first()) {
        return TokenInfo();
    }
    return TokenInfo(result.value("id").toUuid(), result.value("username").toString(), result.value("creationdate").toDateTime(), result.value("devicename").toString());
}

TokenInfo BuiltinUserBackend::tokenInfo(const QUuid &tokenId) const
{
    QString getTokenQuery = QString("SELECT id, username, creationdate, deviceName FROM tokens WHERE id = \"%1\";")
            .arg(tokenId.toString());
    QSqlQuery result = m_db.exec(getTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokenQuery;
        return TokenInfo();
    }

    if (!result.first()) {
        return TokenInfo();
    }
    return TokenInfo(result.value("id").toUuid(), result.value("username").toString(), result.value("creationdate").toDateTime(), result.value("devicename").toString());
}

UserManager::UserError BuiltinUserBackend::removeToken(const QUuid &tokenId)
{
    QString removeTokenQuery = QString("DELETE FROM tokens WHERE id = \"%1\";")
            .arg(tokenId.toString());
    QSqlQuery result = m_db.exec(removeTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Removing token failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << removeTokenQuery;
        return UserManager::UserErrorBackendError;
    }
    if (result.numRowsAffected() != 1) {
        qCWarning(dcUserManager) << "Token not found in DB";
        return UserManager::UserErrorTokenNotFound;
    }

    qCDebug(dcUserManager) << "Token" << tokenId << "removed from DB";
    return UserManager::UserErrorNoError;
}

bool BuiltinUserBackend::verifyToken(const QByteArray &token)
{
    if (!validateToken(token)) {
        qCWarning(dcUserManager) << "Token failed character validation" << token;
        return false;
    }
    QString getTokenQuery = QString("SELECT * FROM tokens WHERE token = \"%1\";")
            .arg(QString::fromUtf8(token));
    QSqlQuery result = m_db.exec(getTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokenQuery;
        return false;
    }
    if (!result.first()) {
        qCDebug(dcUserManager) << "Authorization failed for token" << token;
        return false;
    }
    //qCDebug(dcUserManager) << "Token authorized for user" << result.value("username").toString();
    return true;
}

bool BuiltinUserBackend::pushButtonAuthAvailable() const
{
    return m_pushButtonDBusService->agentAvailable();
}

int BuiltinUserBackend::requestPushButtonAuth(const QString &deviceName)
{
    if (m_pushButtonTransaction.first != -1) {
        qCWarning(dcUserManager()) << "PushButton authentication already in progress for device" << m_pushButtonTransaction.second << ". Cancelling...";
        cancelPushButtonAuth(m_pushButtonTransaction.first);
    }

    qCDebug(dcUserManager()) << "Starting PushButton authentication for device" << deviceName;
    int transactionId = ++m_pushButtonTransactionIdCounter;
    m_pushButtonTransaction = qMakePair<int, QString>(transactionId, deviceName);
    return transactionId;
}

void BuiltinUserBackend::cancelPushButtonAuth(int transactionId)
{
    if (m_pushButtonTransaction.first == -1) {
        qCWarning(dcUserManager()) << "No PushButton transaction in progress. Nothing to cancel.";
        return;
    }
    if (m_pushButtonTransaction.first != transactionId) {
        qCWarning(dcUserManager()) << "PushButton transaction" << transactionId << "not in progress. Cannot cancel.";
        return;
    }
    qCDebug(dcUserManager()) << "Cancelling PushButton transaction for device:" << m_pushButtonTransaction.second;
    emit pushButtonAuthFinished(m_pushButtonTransaction.first, false, QByteArray());
    m_pushButtonTransaction.first = -1;


}

bool BuiltinUserBackend::validateUsername(const QString &username) const
{
    QRegExp validator("[a-zA-Z0-9_\\.+-@]{3,}");
    return validator.exactMatch(username);
}

bool BuiltinUserBackend::validatePassword(const QString &password) const
{
    if (password.length() < 8) {
        return false;
    }
    if (!password.contains(QRegExp("[a-z]"))) {
        return false;
    }
    if (!password.contains(QRegExp("[A-Z]"))) {
        return false;
    }
    if (!password.contains(QRegExp("[0-9]"))) {
        return false;
    }
    return true;
}

bool BuiltinUserBackend::validateToken(const QByteArray &token) const
{
    QRegExp validator(QRegExp("(^[a-zA-Z0-9_\\.+-/=]+$)"));
    return validator.exactMatch(token);
}

void BuiltinUserBackend::dumpDBError(const QString &message)
{
    qCCritical(dcUserManager) << message << "Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
}

bool BuiltinUserBackend::initDB()
{
    m_db.close();

    if (!m_db.open()) {
        dumpDBError("Can't open user database. Init failed.");
        return false;
    }

    int currentVersion = -1;
    int newVersion = 1;
    if (m_db.tables().contains("metadata")) {
        QSqlQuery query = m_db.exec("SELECT data FROM metadata WHERE `key` = 'version';");
        if (query.next()) {
            currentVersion = query.value("data").toInt();
        }
    }

    if (!m_db.tables().contains("users")) {
        qCDebug(dcUserManager()) << "Empty user database. Setting up metadata...";
        m_db.exec("CREATE TABLE users (username VARCHAR(40) UNIQUE PRIMARY KEY, email VARCHAR(40), displayName VARCHAR(40), password VARCHAR(100), salt VARCHAR(100), scopes TEXT);");
        if (m_db.lastError().isValid()) {
            dumpDBError("Error initializing user database (table users).");
            m_db.close();
            return false;
        }
    } else {
        if (currentVersion < 1) {
            m_db.exec("ALTER TABLE users ADD COLUMN scopes TEXT;");
            if (m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }
            // Migrated existing users from before multiuser support are admins by default
            QSqlQuery query(m_db);
            query.prepare("UPDATE users SET scopes = ?;");
            query.addBindValue(Types::scopesToStringList(Types::PermissionScopeAdmin).join(','));
            query.exec();

            if (query.lastError().isValid()) {
                dumpDBError("Error migrating user database (updating existing users).");
                m_db.close();
                return false;
            }

            m_db.exec("ALTER TABLE users ADD COLUMN email VARCHAR(40);");
            if (m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }
            m_db.exec("ALTER TABLE users ADD COLUMN displayName VARCHAR(40);");
            if (m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }

            // Up until schema 1, username was an email. Copy it to initialize the email field.
            m_db.exec("UPDATE users SET email = username;");
            if (m_db.lastError().isValid()) {
                dumpDBError("Error migrating user database (table users).");
                m_db.close();
                return false;
            }
            currentVersion = 1;
        }
    }

    if (!m_db.tables().contains("tokens")) {
        qCDebug(dcUserManager()) << "Empty user database. Setting up metadata...";
        m_db.exec("CREATE TABLE tokens (id VARCHAR(40) UNIQUE, username VARCHAR(40), token VARCHAR(100) UNIQUE, creationdate DATETIME, devicename VARCHAR(40));");
        if (m_db.lastError().isValid()) {
            dumpDBError("Error initializing user database (table tokens).");
            m_db.close();
            return false;
        }
    }

    if (m_db.tables().contains("metadata")) {
        if (currentVersion < newVersion) {
            m_db.exec(QString("UPDATE metadata SET data = %1 WHERE `key` = 'version')").arg(newVersion));
            if (m_db.lastError().isValid()) {
                dumpDBError("Error updating up user database schema version!");
                m_db.close();
                return false;
            }
            qCInfo(dcUserManager()) << "Successfully migrated user database.";
        }
    } else {
        m_db.exec("CREATE TABLE metadata (`key` VARCHAR(10), data VARCHAR(40));");
        if (m_db.lastError().isValid()) {
            dumpDBError("Error setting up user database (table metadata)!");
            m_db.close();
            return false;
        }
        m_db.exec(QString("INSERT INTO metadata (`key`, `data`) VALUES ('version', %1);").arg(newVersion));
        if (m_db.lastError().isValid()) {
            dumpDBError("Error setting up user database (setting version metadata)!");
            m_db.close();
            return false;
        }
        qCInfo(dcUserManager()) << "Successfully initialized user database.";
    }


    // Migration from before 1.0:
    // Push button tokens were given out without an explicit user name
    // If we have push button tokens (userId "") but no explicit user, let's create it as admin
    // Users without valid username will have password login disabled.
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tokens WHERE username = \"\";");
    query.exec();
    if (query.lastError().type() == QSqlError::NoError && query.next()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM users WHERE username = \"\";");
        query.exec();
        if (!query.next()) {
            qCDebug(dcUserManager()) << "Tokens existing but no user. Creating token admin user";
            QSqlQuery query(m_db);
            query.prepare("INSERT INTO users(username, email, displayName, password, salt, scopes) values(?, ?, ?, ?, ?, ?);");
            query.addBindValue("");
            query.addBindValue("");
            query.addBindValue("Admin");
            query.addBindValue("");
            query.addBindValue("");
            query.addBindValue(Types::scopeToString(Types::PermissionScopeAdmin));
            query.exec();
            if (query.lastError().type() != QSqlError::NoError) {
                qCWarning(dcUserManager) << "Error creating push button user:" << query.lastError().databaseText() << query.lastError().driverText();
            }
        }
    }


    qCDebug(dcUserManager()) << "User database initialized successfully.";
    return true;
}

void BuiltinUserBackend::rotate(const QString &dbName)
{
    int index = 1;
    while (QFileInfo(QString("%1.%2").arg(dbName).arg(index)).exists()) {
        index++;
    }
    qCDebug(dcUserManager()) << "Backing up old database file to" << QString("%1.%2").arg(dbName).arg(index);
    QFile f(dbName);
    if (!f.rename(QString("%1.%2").arg(dbName).arg(index))) {
        qCWarning(dcUserManager()) << "Error backing up old database.";
    } else {
        qCDebug(dcUserManager()) << "Successfully moved old database";
    }
}

void BuiltinUserBackend::onPushButtonPressed()
{
    if (m_pushButtonTransaction.first == -1) {
        qCDebug(dcUserManager()) << "PushButton pressed without a client waiting for it. Ignoring the signal.";
        return;
    }

    // Creating a user without username and password. It won't be able to log in via user/password
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM users WHERE username = \"\";");
    query.exec();
    if (!query.next()) {
        qCDebug(dcUserManager()) << "Creating token admin user";
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO users(username, password, salt, scopes) values(?, ?, ?, ?);");
        query.addBindValue("");
        query.addBindValue("");
        query.addBindValue("");
        query.addBindValue(Types::scopeToString(Types::PermissionScopeAdmin));
        query.exec();
        if (query.lastError().type() != QSqlError::NoError) {
            qCWarning(dcUserManager) << "Error creating push button user:" << query.lastError().databaseText() << query.lastError().driverText();
        }
    }

    QByteArray token = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QString storeTokenQuery = QString("INSERT INTO tokens(id, username, token, creationdate, devicename) VALUES(\"%1\", \"%2\", \"%3\", \"%4\", \"%5\");")
            .arg(QUuid::createUuid().toString())
            .arg("")
            .arg(QString::fromUtf8(token))
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(m_pushButtonTransaction.second);

    m_db.exec(storeTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager()) << "Error storing token in DB:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        qCWarning(dcUserManager()) << "PushButton Auth failed.";
        emit pushButtonAuthFinished(m_pushButtonTransaction.first, false, QByteArray());
    } else {
        qCDebug(dcUserManager()) << "PushButton Auth succeeded.";
        emit pushButtonAuthFinished(m_pushButtonTransaction.first, true, token);
    }

    m_pushButtonTransaction.first = -1;
}

}
