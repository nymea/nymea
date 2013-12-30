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




//    // create 433.92 MHz sender
//    m_sender = new RadioSender(this);
//    m_sender->setFrequency(RadioSender::RF433MHz);
//    m_sender->setLineCode(RadioSender::SWITCH);
//    m_sender->setPulseLength(320);
////    //m_sender->sendBin("000000000000010101010001");

//    // create 433.92 MHz receiver
//    m_reciver = new RadioReciver(this);
//    m_reciver->setFrequency(RadioReciver::RF433MHz);
//    m_reciver->setPin(2);
//    m_reciver->enableReceiver();

}
