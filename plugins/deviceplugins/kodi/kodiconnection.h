#ifndef KODICONNECTION_H
#define KODICONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>

class KodiConnection : public QObject
{
    Q_OBJECT
public:
    explicit KodiConnection(const QHostAddress &hostAddress, const int &port = 9090, QObject *parent = 0);

    void connectToKodi();
    void disconnectFromKodi();

    QHostAddress hostAddress() const;
    int port() const;

private:
    QHostAddress m_hostAddress;
    int m_port;
    int m_id;



    QTcpSocket *m_socket;

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void readData();

signals:
    void connectionStateChanged(const bool &connected);
    void dataReady(const QByteArray &data);

public slots:
    void sendData(const QString &method, const QVariantMap &params = QVariantMap());

};

#endif // KODICONNECTION_H
