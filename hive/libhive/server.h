#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    
private:
    QList<QTcpServer*> m_serverList;
    QList<QTcpSocket*> m_clientList;


signals:
    
public slots:
    
private slots:
    void incomingConnection();
    void readPackage();
};

#endif // SERVER_H
