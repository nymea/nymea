#ifndef HIVECORE_H
#define HIVECORE_H

#include <QObject>
//#include "server.h"
//#include "devicemanager.h"
//#include <jsonhandler.h>
//#include "radio/radioreciver.h"
//#include "radio/radiosender.h"

class JsonRPCServer;
class Device;
class DeviceClass;

class HiveCore : public QObject
{
    Q_OBJECT
public:
    static HiveCore* instance();
    
    QList<Device*> devices() const;

private:
    explicit HiveCore(QObject *parent = 0);
    static HiveCore *s_instance;

    JsonRPCServer *m_jsonServer;

//    Server *m_server;
//    RadioReciver *m_reciver;
//    RadioSender *m_sender;
//    DeviceManager *m_deviceManager;
//    JsonHandler *m_jsonHandler;

signals:
    
public slots:
    
private:
    QList<DeviceClass*> m_supportedDevices;
    QList<Device*> m_devices;
};

#endif // HIVECORE_H
