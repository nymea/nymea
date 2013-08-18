#include "hivecore.h"
#include <QDebug>

HiveCore::HiveCore(QObject *parent) :
    QObject(parent)
{
    // start the server
    m_server = new Server(this);
    m_server->startServer();

    m_sender = new RadioSender(this);

    QByteArray data("11101101");
    QDataStream stream(&data, QIODevice::ReadOnly);
    qint8 dataInt;
    stream >> dataInt;

    qDebug() << data << dataInt << data.toInt(0,2);

    m_reciver = new RadioReciver(this);
    m_reciver->setFrequence(RadioReciver::RC433MHz);
    m_reciver->enableReceiver();

    DeviceManager deviceManager;

}
