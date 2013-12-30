#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    
private:
    QList<QTcpServer*> m_serverList;
    QList<QTcpSocket*> m_clientList;


signals:
    void jsonDataAvailable(const QByteArray &data);
    
private slots:
    void newClientConnected();
    void readPackage();
    void clientDisconnected();

public slots:
    bool startServer();
    bool stopServer();
    void sendToAll(QByteArray data);
    

};

#endif // TCPSERVER_H
