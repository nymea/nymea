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

    void actionExecuted(const ActionTypeId &actionTypeId);

signals:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &eventTypeId);

private slots:
    void readClient();
    void discardClient();

private:
    QString generateHeader();
    QString generateWebPage();

private:
    bool disabled;

    DevicePlugin *m_plugin;
    Device *m_device;

    QList<QPair<ActionTypeId, QDateTime> > m_actionList;
};

#endif // HTTPDAEMON_H
