#ifndef JANUSCONNECTOR_H
#define JANUSCONNECTOR_H

#include <QObject>
#include <QLocalSocket>
#include <QTimer>

class JanusConnector : public QObject
{
    Q_OBJECT
public:
    class WebRtcSession {
    public:
        QString sessionId; // This should be unique but persistent for each remote device
        qint64 janusSessionId = -1; // the session id for the janus session.
        qint64 janusChannelId = -1;
        bool connectedToJanus = false;

        QVariantMap offer;
        bool offerSent = false;
        QVariantList trickles;
        QVariantMap webRtcUp;
        bool webRtcConnected = false;

        bool matchJanusSessionId(qint64 id) {
            return (janusSessionId == id) || (janusSessionId == id +1) || (janusSessionId == id -1);
        }
    };

    explicit JanusConnector(QObject *parent = nullptr);


    void sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &message);

signals:
    void connected();
    void webRtcHandshakeMessageReceived(const QString &sessionId, const QVariantMap &message);

private slots:
    void onDisconnected();
    void onError(QLocalSocket::LocalSocketError socketError);
    void onTextMessageReceived(const QString &message);
    void onReadyRead();
    void heartbeat();
    void processQueue();

private:
    QHash<QString, WebRtcSession*> m_pendingRequests;

    bool connectToJanus();
    void createSession(WebRtcSession *session);
    void createChannel(WebRtcSession *session);
    void writeToJanus(const QByteArray &data);


private:

    QLocalSocket *m_socket = nullptr;
    QTimer m_socketTimeoutTimer;

    QHash<QString, WebRtcSession*> m_sessions;

    QStringList m_wantedAcks;
};

QDebug operator<<(QDebug debug, const JanusConnector::WebRtcSession &session);
QDebug operator<<(QDebug debug, JanusConnector::WebRtcSession *session);

#endif // JANUSCONNECTOR_H
