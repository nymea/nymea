#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    
private:
    QList<QTcpServer*> m_serverList;
    QList<QTcpSocket*> m_clientList;


signals:
    void dataLineAvailable(const QByteArray &dataLine);
    
private slots:
    void newClientConnected();
    void readPackage();
    void clientDisconnected();

public slots:
    bool startServer();
    bool stopServer();
    void sendToAll(QByteArray data);
    

};

#endif // SERVER_H
