#include "cloudmanager.h"
#include "guhcore.h"

#include <QNetworkSession>
#include <QNetworkConfigurationManager>

CloudManager::CloudManager(QObject *parent) : QObject(parent)
{
    m_awsConnector = new AWSConnector(this);
    connect(m_awsConnector, &AWSConnector::subscriptionReceived, this, &CloudManager::subscriptionReceived);

    // Extract the machine id so we have a unique identifier for this machine
    // TODO: this only works for debian based systems, perhaps we should find something more general
    QFile f("/etc/machine-id");
    if (f.open(QFile::ReadOnly)) {
        m_deviceId = QString::fromLatin1(f.readAll()).trimmed();
        qCDebug(dcCloud()) << "Device ID is:" << m_deviceId;
        setEnabled(true);
        m_awsConnector->subscribe({"pair/response"});
    } else {
        qWarning(dcCloud()) << "Failed to open /etc/machine-id for reading. Cloud connection will not work.";
    }

    connect(GuhCore::instance()->networkManager(), &NetworkManager::stateChanged, this, &CloudManager::onlineStateChanged);
}

void CloudManager::setServerUrl(const QString &serverUrl)
{
    m_serverUrl = serverUrl;
}

void CloudManager::setDeviceId(const QString &deviceId)
{
    m_deviceId = deviceId;
}

void CloudManager::setClientCertificates(const QString &caCertificate, const QString &clientCertificate, const QString &clientCertificateKey)
{
    m_caCertificate = caCertificate;
    m_clientCertificate = clientCertificate;
    m_clientCertificateKey = clientCertificateKey;
}

bool CloudManager::enabled() const
{
    return m_enabled;
}

void CloudManager::setEnabled(bool enabled)
{
    if (enabled) {
        bool missingConfig = false;
        if (m_deviceId.isEmpty()) {
            qCWarning(dcCloud()) << "Don't have a unique device ID (/etc/machine-id).";
            missingConfig = true;
        }

        if (GuhCore::instance()->configuration()->cloudServerUrl().isEmpty()) {
            qCWarning(dcCloud()) << "Cloud server URL not set in configuration.";
            missingConfig = true;
        }
        if (GuhCore::instance()->configuration()->cloudCertificate().isEmpty()) {
            qCWarning(dcCloud()) << "Cloud certificate not set in configuration.";
            missingConfig = true;
        }
        if (GuhCore::instance()->configuration()->cloudCertificateKey().isEmpty()) {
            qCWarning(dcCloud()) << "Cloud certificate key not set in configuration.";
            missingConfig = true;
        }
        if (GuhCore::instance()->configuration()->cloudCertificateCA().isEmpty()) {
            qCWarning(dcCloud()) << "Cloud certificate CA not set in configuration.";
            missingConfig = true;
        }
        if (missingConfig) {
            qCWarning(dcCloud()) << "Cloud configuration incomplete. Not enabling cloud connection.";
            return;
        }

        m_enabled = true;
        if (!m_awsConnector->isConnected() && GuhCore::instance()->networkManager()->state() == NetworkManager::NetworkManagerStateConnectedGlobal) {
            connect2aws();
        }
    }
}

int CloudManager::pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoId)
{
    QVariantMap map;
    map.insert("id", ++m_id);
    map.insert("idToken", idToken);
    map.insert("authToken", authToken);
    map.insert("cognitoId", cognitoId);
    m_awsConnector->publish("pair", map);
    return m_id;
}

void CloudManager::connect2aws()
{
    m_awsConnector->connect2AWS(GuhCore::instance()->configuration()->cloudServerUrl(),
                                m_deviceId,
                                GuhCore::instance()->configuration()->cloudCertificateCA(),
                                GuhCore::instance()->configuration()->cloudCertificate(),
                                GuhCore::instance()->configuration()->cloudCertificateKey()
                                );
}

void CloudManager::onlineStateChanged()
{
    qWarning() << "online state changed" << GuhCore::instance()->networkManager()->state();
    if (GuhCore::instance()->networkManager()->state() == NetworkManager::NetworkManagerStateConnectedGlobal) {
        if (m_enabled && !m_awsConnector->isConnected()) {
            connect2aws();
        }
    }
}

void CloudManager::subscriptionReceived(const QString &topic, const QVariantMap &message)
{
    qCDebug(dcCloud()) << "subscription reveiv ed" << topic << message;
    if (topic == "pair/response") {
        emit pairingReply(message.value("id").toInt(), message.value("status").toInt());
    }
}
