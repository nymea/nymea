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

    void sendResponse(int clientId, const QByteArray &data);

/************** Used for testing **************************/
    static QList<MockTcpServer*> servers();
    void injectData(int clientId, const QByteArray &data);
signals:
    void outgoingData(int clientId, const QByteArray &data);
/************** Used for testing **************************/

signals:
    void jsonDataAvailable(int clientId, const QByteArray &data);

public slots:
    bool startServer();
    bool stopServer();
    void sendToAll(QByteArray data);

private:
    static QList<MockTcpServer*> s_allServers;
};

#endif // TCPSERVER_H

