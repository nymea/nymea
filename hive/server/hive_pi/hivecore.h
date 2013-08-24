#ifndef HIVECORE_H
#define HIVECORE_H

#include <QObject>
#include "server.h"
#include "devicemanager.h"
#include "radio/radioreciver.h"
#include "radio/radiosender.h"

class HiveCore : public QObject
{
    Q_OBJECT
public:
    explicit HiveCore(QObject *parent = 0);
    
private:
    Server *m_server;
    RadioReciver *m_reciver;
    RadioSender *m_sender;
    DeviceManager *m_deviceManager;
signals:
    
public slots:
    
};

#endif // HIVECORE_H
