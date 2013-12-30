#include "hivecore.h"
#include "jsonrpcserver.h"

#include "device.h"
#include "deviceclass.h"

#include <QDebug>

HiveCore* HiveCore::s_instance = 0;

HiveCore *HiveCore::instance()
{
    if (!s_instance) {
        s_instance = new HiveCore();
    }
    return s_instance;
}

QList<Device *> HiveCore::devices() const
{
    return m_devices;
}

HiveCore::HiveCore(QObject *parent) :
    QObject(parent)
{
    // create a fake device
    m_devices.append(new Device(this));


    // start the server
    m_jsonServer = new JsonRPCServer(this);

    m_radio433 = new Radio433(this);

    //==============================================
    int pulseLenght = 350;
    QList<int> signal;
    //sync
    signal.append(pulseLenght);
    signal.append(pulseLenght*31);

    //==============================================
    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);
    //==============================================
    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);


    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);


    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);
    //==============================================

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 1
    signal.append(pulseLenght*3);
    signal.append(pulseLenght);

    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    // 0
    signal.append(pulseLenght);
    signal.append(pulseLenght*3);
    //==============================================

    qDebug() << "sendsignal";

    m_radio433->sendData(signal);
    m_radio433->sendData(signal);
    m_radio433->sendData(signal);
    m_radio433->sendData(signal);




}
