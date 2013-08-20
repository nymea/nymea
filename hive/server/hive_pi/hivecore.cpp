#include "hivecore.h"
#include <QDebug>

HiveCore::HiveCore(QObject *parent) :
    QObject(parent)
{
    // start the server
    m_server = new Server(this);
    m_server->startServer();

    // create Sender
    m_sender = new RadioSender(this);
    m_sender->setFrequency(RadioSender::RF433MHz);
    m_sender->setLineCode(RadioSender::THERMOMETER);
    m_sender->setPulseLength(250);
    m_sender->sendBin("010100101000101111110110");
    m_sender->sendBin("010100101000101111110110");

    // create 433.92 MHz receiver
    m_reciver = new RadioReciver(this);
    m_reciver->setFrequency(RadioReciver::RF433MHz);
    m_reciver->enableReceiver();
    m_sender->sendBin("010100101000101111110110");



}
