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
    bool sendKeepAliveMessage(const QString &sessionId);

signals:
    void connected();
    void webRtcHandshakeMessageReceived(const QString &sessionId, const QVariantMap &message);

private slots:
    void onDisconnected();
    void onError(QLocalSocket::LocalSocketError socketError);
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

    QHash<QString, WebRtcSession*> m_sessions;

    QStringList m_wantedAcks;
};

QDebug operator<<(QDebug debug, const JanusConnector::WebRtcSession &session);
QDebug operator<<(QDebug debug, JanusConnector::WebRtcSession *session);

#endif // JANUSCONNECTOR_H
