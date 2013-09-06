#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)


public:
    explicit Client(QObject *parent = 0);

    enum Request {
        RequestAddDevice,
        RequestEditDevice,
        RequestRemoveDevice
    };

    bool isConnected();

private:
    QTcpSocket *m_tcpSocket;
    QString m_tcpBuffer;

    bool m_connectionStatus;
    int m_commandId;

    QMap<int, Request> m_requestMap;


signals:
    void dataAvailable(const QByteArray &data);
    void connectionChanged();


private slots:
    void connectionError(QAbstractSocket::SocketError error);
    void readData();
    void connected();
    void disconneted();

    void processData(const QByteArray &data);
    void handleResponse(const QVariantMap &rsp);
    void handleSignal(const QVariantMap &signal);


public slots:
    void connectToHost(QString ipAddress, QString port);
    void disconnectFromHost();
    void sendData(QByteArray data);
};

#endif // CLIENT_H
