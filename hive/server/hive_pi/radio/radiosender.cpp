#include "radiosender.h"

#include <QDebug>
#include "wiringPi.h"

RadioSender::RadioSender(QObject *parent) :
    QObject(parent)
{
    m_pulseLength = 350;

}

void RadioSender::sendSync()
{

}

void RadioSender::transmit(int high, int low)
{

}

void RadioSender::sendBin(QString codeBin)
{

    if(wiringPiSetup() == -1){
        qDebug() << "ERROR: GPIO setup failed.";
    }
    qDebug() << "GPIO setup ok.";
    pinMode(0,OUTPUT);



}

void RadioSender::setPulseLength(int pulseLength)
{
    m_pulseLength = pulseLength;
}
