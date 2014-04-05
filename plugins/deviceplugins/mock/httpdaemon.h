#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include "typeutils.h"

#include <QTcpServer>
#include <QUuid>
#include <QDateTime>

class Device;
class DevicePlugin;

class HttpDaemon : public QTcpServer
{
    Q_OBJECT
public:
    HttpDaemon(Device *device, DevicePlugin* parent = 0);

    void incomingConnection(qintptr socket) override;

    void actionExecuted(const QUuid&actionTypeId);

signals:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &eventTypeId);

private slots:
    void readClient();
    void discardClient();

private:
    QString generateWebPage();

private:
    bool disabled;

    DevicePlugin *m_plugin;
    Device *m_device;

    QList<QPair<QUuid, QDateTime> > m_actionList;
};

#endif // HTTPDAEMON_H
