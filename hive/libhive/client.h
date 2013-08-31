#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class Client : public QObject
{
    Q_OBJECT


public:
    explicit Client(QObject *parent = 0);
    
private:
    QTcpSocket *m_tcpSocket;
    QString m_tcpBuffer;


signals:
    void connected();
    void jsonDataAvailable(const QByteArray &data);


private slots:
    void connectionError(QAbstractSocket::SocketError error);
    void readData();
    void connectedToHost();
    
public slots:
    void connectToHost(QString ipAddress, QString port);
    void disconnectFromHost();
    void sendData(QByteArray data);
};

#endif // CLIENT_H
