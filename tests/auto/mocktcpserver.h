#ifndef MOCKTCPSERVER_H
#define MOCKTCPSERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QDebug>

class MockTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MockTcpServer(QObject *parent = 0);
    ~MockTcpServer();

    void sendData(const QUuid &clientId, const QByteArray &data);

/************** Used for testing **************************/
    static QList<MockTcpServer*> servers();
    void injectData(const QUuid &clientId, const QByteArray &data);
signals:
    void outgoingData(const QUuid &clientId, const QByteArray &data);
/************** Used for testing **************************/

signals:
    void dataAvailable(const QUuid &clientId, const QByteArray &data);

public slots:
    bool startServer();
    bool stopServer();
    void sendToAll(QByteArray data);

private:
    static QList<MockTcpServer*> s_allServers;
};

#endif // TCPSERVER_H

