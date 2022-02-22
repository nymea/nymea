#include "maveouserbackend.h"
#include "mqttclient.h"
#include "loggingcategories.h"

#include <QSslConfiguration>
#include <QSslKey>
#include <QFile>

NYMEA_LOGGING_CATEGORY(dcMaveoUserBackend, "MaveoUserBackend")

MaveoUserBackend::MaveoUserBackend(QObject *parent)
    : UserBackend{parent}
{
    connect2Aws();
}

bool MaveoUserBackend::initRequired() const
{
    return true;
}

UserManager::UserError MaveoUserBackend::createUser(const QString &username, const QString &password, const QString &email, const QString &displayName, Types::PermissionScopes scopes)
{
    Q_UNUSED(username)
    Q_UNUSED(password)
    Q_UNUSED(email)
    Q_UNUSED(displayName)
    Q_UNUSED(scopes)
    return UserManager::UserErrorBackendError;
}

UserManager::UserError MaveoUserBackend::changePassword(const QString &username, const QString &newPassword)
{
    Q_UNUSED(username)
    Q_UNUSED(newPassword)
    return UserManager::UserErrorBackendError;
}

UserManager::UserError MaveoUserBackend::removeUser(const QString &username)
{
    Q_UNUSED(username)
    return UserManager::UserErrorBackendError;
}

UserManager::UserError MaveoUserBackend::setUserScopes(const QString &username, Types::PermissionScopes scopes)
{
    Q_UNUSED(username)
    Q_UNUSED(scopes)
    return UserManager::UserErrorBackendError;
}

UserInfo MaveoUserBackend::userInfo(const QString &username) const
{
    Q_UNUSED(username)
    return UserInfo();
}

UserManager::UserError MaveoUserBackend::setUserInfo(const QString &username, const QString &email, const QString &displayName)
{
    Q_UNUSED(username)
    Q_UNUSED(email)
    Q_UNUSED(displayName)
    return UserManager::UserErrorBackendError;
}

UserInfoList MaveoUserBackend::users() const
{
    return UserInfoList();
}

QByteArray MaveoUserBackend::authenticate(const QString &username, const QString &password, const QString &deviceName)
{
    Q_UNUSED(username)
    Q_UNUSED(password)
    Q_UNUSED(deviceName)
    return QByteArray();
}

QList<TokenInfo> MaveoUserBackend::tokens(const QString &username) const
{
    Q_UNUSED(username)
    return QList<TokenInfo>();
}

TokenInfo MaveoUserBackend::tokenInfo(const QByteArray &token) const
{
    Q_UNUSED(token)
    return TokenInfo();
}

TokenInfo MaveoUserBackend::tokenInfo(const QUuid &tokenId) const
{
    Q_UNUSED(tokenId)
    return TokenInfo();
}

UserManager::UserError MaveoUserBackend::removeToken(const QUuid &tokenId)
{
    Q_UNUSED(tokenId)
    return UserManager::UserErrorBackendError;
}

bool MaveoUserBackend::verifyToken(const QByteArray &token)
{
    Q_UNUSED(token)
    return false;
}

void MaveoUserBackend::connect2Aws()
{
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    QFile certFile("/home/micha/Develop/cloud/certs-testing/guh-cloud.pem");
    certFile.open(QFile::ReadOnly);
    QSslCertificate certificate(certFile.readAll());

    QFile keyFile("/home/micha/Develop/cloud/certs-testing/guh-cloud.key");
    keyFile.open(QFile::ReadOnly);
    QSslKey key(keyFile.readAll(), QSsl::Rsa);

    sslConfig.setLocalCertificate(certificate);
    sslConfig.setPrivateKey(key);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);

    QFile caCertFile("/home/micha/Develop/cloud/certs-testing/aws-certification-authority.crt");
    caCertFile.open(QFile::ReadOnly);
    QSslCertificate caCertificate(caCertFile.readAll());

    sslConfig.setCaCertificates({caCertificate});

    m_client = new MqttClient(certificate.serialNumber(), this);
    m_client->setKeepAlive(30);
//    m_client->setAutoReconnect(false);
    qCDebug(dcMaveoUserBackend()).nospace().noquote() << "Connecting MQTT to " << "iot-eu.marantec-cloud.de" << " as " << certificate.serialNumber() << " with certificate ";// << getCertificateFingerprint(certificate);
    m_client->connectToHost("iot-eu.marantec-cloud.de", 8883, true, true, sslConfig);

    connect(m_client, &MqttClient::connected, this, &MaveoUserBackend::onConnected);
    connect(m_client, &MqttClient::disconnected, this, &MaveoUserBackend::onDisconnected);
    connect(m_client, &MqttClient::error, this, [this](const QAbstractSocket::SocketError error){
//        m_connectTimer.stop();
        qCWarning(dcMaveoUserBackend()) << "An error happened in the MQTT transport" << error;
//        // In order to also call onDisconnected (and start the reconnect timer) even when we have never been connected
//        // we'll call it here. However, that might cause onDisconnected to be called twice. Let's prevent that.
//        disconnect(m_client, &MqttClient::disconnected, this, &AWSConnector::onDisconnected);
//        onDisconnected();
    });

//    connect(m_client, &MqttClient::subscribed, this, &AWSConnector::onSubscribed);
//    connect(m_client, &MqttClient::publishReceived, this, &AWSConnector::onPublishReceived);
//    connect(m_client, &MqttClient::published, this, &AWSConnector::onPublished);

}

void MaveoUserBackend::onConnected()
{
    qCDebug(dcMaveoUserBackend) << "Connected to cloud!";
}

void MaveoUserBackend::onDisconnected()
{
    qCDebug(dcMaveoUserBackend()) << "Disconnected from cloud";
}
