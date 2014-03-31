#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QTcpServer>

class Device;

class HttpDaemon : public QTcpServer
{
    Q_OBJECT
public:
    HttpDaemon(Device *device, QObject* parent = 0);

    void incomingConnection(qintptr socket) override;

    void pause();

    void resume();

signals:
    void triggerEvent(int id);

private slots:
    void readClient();
    void discardClient();

private:
    bool disabled;

    Device *m_device;
};

#endif // HTTPDAEMON_H
