#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QObject>

class TcpServer;

class JsonRPCServer: public QObject
{
    Q_OBJECT
public:
    JsonRPCServer(QObject *parent = 0);

signals:

private slots:
    void processData(const QByteArray &jsonData);

private:
    TcpServer *m_tcpServer;
};

#endif
