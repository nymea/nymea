#ifndef HIVECORE_H
#define HIVECORE_H

#include <QObject>
#include <radio433.h>

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
    Radio433 *m_radio433;

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
