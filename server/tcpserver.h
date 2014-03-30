#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUuid>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    
    void sendData(const QUuid &clientId, const QByteArray &data);
    void sendData(const QList<QUuid> &clients, const QByteArray &data);

private:
    QHash<QUuid, QTcpServer*> m_serverList;
    QHash<QUuid, QTcpSocket*> m_clientList;

signals:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QByteArray &data);
    
private slots:
    void newClientConnected();
    void readPackage();
    void slotClientDisconnected();

public slots:
    bool startServer();
    bool stopServer();
};

#endif // TCPSERVER_H
