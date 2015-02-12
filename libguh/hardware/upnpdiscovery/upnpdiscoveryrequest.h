#ifndef UPNPDISCOVERYREQUEST_H
#define UPNPDISCOVERYREQUEST_H

#include <QObject>
#include <QDebug>
#include "upnpdiscovery.h"
#include "upnpdevicedescriptor.h"
#include "typeutils.h"

class UpnpDiscovery;

class UpnpDiscoveryRequest : public QObject
{
    Q_OBJECT
public:
    explicit UpnpDiscoveryRequest(UpnpDiscovery *upnpDiscovery, PluginId pluginId, QString searchTarget, QString userAgent);

    void discover();
    void addDeviceDescriptor(const UpnpDeviceDescriptor &deviceDescriptor);
    QNetworkRequest createNetworkRequest(UpnpDeviceDescriptor deviveDescriptor);
    QList<UpnpDeviceDescriptor> deviceList() const;

    PluginId pluginId() const;
    QString searchTarget() const;
    QString userAgent() const;

private:
    UpnpDiscovery *m_upnpDiscovery;
    QTimer *m_timer;
    PluginId m_pluginId;
    QString m_searchTarget;
    QString m_userAgent;

    QList<UpnpDeviceDescriptor> m_deviceList;

signals:
    void discoveryTimeout();

public slots:

};

#endif // UPNPDISCOVERYREQUEST_H
