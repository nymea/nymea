#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include "awsconnector.h"

#include <QObject>
#include <QTimer>
#include <QNetworkSession>

class CloudManager : public QObject
{
    Q_OBJECT
public:
    explicit CloudManager(QObject *parent = nullptr);

    void setServerUrl(const QString &serverUrl);
    void setDeviceId(const QString &deviceId);
    void setClientCertificates(const QString &caCertificate, const QString &clientCertificate, const QString &clientCertificateKey);

    bool enabled() const;
    void setEnabled(bool enabled);

    int pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoId);

signals:
    void pairingReply(int pairingTransactionId, int status);

private:
    void connect2aws();

private slots:
    void onlineStateChanged();
    void subscriptionReceived(const QString &topic, const QVariantMap &message);

private:
    QNetworkSession *m_networkSession;
    QTimer m_reconnectTimer;
    bool m_enabled = false;
    AWSConnector *m_awsConnector = nullptr;
    int m_id = 0; // id for transactions. e.g. pairDevice

    QString m_serverUrl;
    QString m_deviceId;
    QString m_caCertificate;
    QString m_clientCertificate;
    QString m_clientCertificateKey;
};

#endif // CLOUDMANAGER_H
