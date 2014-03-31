#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QTcpServer>

class Device;
class DevicePlugin;

class HttpDaemon : public QTcpServer
{
    Q_OBJECT
public:
    HttpDaemon(Device *device, DevicePlugin* parent = 0);

    void incomingConnection(qintptr socket) override;

    void pause();

    void resume();

signals:
    void setState(const QUuid &stateTypeId, const QVariant &value);
    void triggerEvent(const QUuid &eventTypeId);

private slots:
    void readClient();
    void discardClient();

private:
    QString generateWebPage();

private:
    bool disabled;

    DevicePlugin *m_plugin;
    Device *m_device;
};

#endif // HTTPDAEMON_H
