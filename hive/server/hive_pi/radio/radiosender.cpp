#include "radiosender.h"

#include <QDebug>
#include "wiringPi.h"

RadioSender::RadioSender(QObject *parent) :
    QObject(parent)
{
    // set default pulselenght
    m_pulseLength = 350;

    // set default radio frequency module
    m_frequenze = RadioSender::RF433MHz;

}

void RadioSender::sendSync()
{
    // sync for UNIPOLAR:
    if(m_lineCode == RadioSender::UNIPOLAR){

    }
    // sync for MANCHESTER:
    if(m_lineCode == RadioSender::MANCHESTER){

    }
    // sync for differential MANCHESTER:
    if(m_lineCode == RadioSender::DMANCHESTER){

    }
    // sync for REMOTE: 1 high 31 low
    if(m_lineCode == RadioSender::REMOTE){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength);
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength*31);
    }
    // sync for THERMOMETER: 1 high 31 low
    if(m_lineCode == RadioSender::THERMOMETER){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength);
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength*31);
    }
    // sync for WEATHERSTATION:
    if(m_lineCode == RadioSender::WEATHERSTATION){

    }

}

void RadioSender::send0()
{
    // 0 in UNIPOLAR encoding
    if(m_lineCode == RadioSender::UNIPOLAR){
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength);
    }
    // 0 in MANCHESTER encoding
    if(m_lineCode == RadioSender::MANCHESTER){

    }
    // 0 in differential MANCHESTER encoding
    if(m_lineCode == RadioSender::DMANCHESTER){

    }
    // 0 in REMOTE encoding
    if(m_lineCode == RadioSender::REMOTE){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength);
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength*3);
    }
    // 0 in THERMOMETER encoding
    if(m_lineCode == RadioSender::THERMOMETER){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength*2);
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength*8);
    }
    // 0 in WEATHERSTATION encoding
    if(m_lineCode == RadioSender::WEATHERSTATION){

    }
}

void RadioSender::send1()
{
    // 1 in UNIPOLAR encoding
    if(m_lineCode == RadioSender::UNIPOLAR){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength);
    }
    // 1 in MANCHESTER encoding
    if(m_lineCode == RadioSender::MANCHESTER){

    }
    // 1 in differential MANCHESTER encoding
    if(m_lineCode == RadioSender::DMANCHESTER){

    }
    // 1 in REMOTE encoding
    if(m_lineCode == RadioSender::REMOTE){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength*3);
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength);
    }
    // 1 in THERMOMETER encoding
    if(m_lineCode == RadioSender::THERMOMETER){
        digitalWrite(m_pin,HIGH);
        delayMicroseconds(m_pulseLength*2);
        digitalWrite(m_pin,LOW);
        delayMicroseconds(m_pulseLength*16);
    }
    // 1 in WEATHERSTATION encoding
    if(m_lineCode == RadioSender::WEATHERSTATION){

    }
}



void RadioSender::sendBin(QByteArray codeBin)
{
    qDebug() << "send" << codeBin;
    for(int i = 0; i < 8; i++){
        for(int i = 0; i < codeBin.length(); i++){
            if(codeBin.at(i) == '0'){
                send0();
            }
            if(codeBin.at(i) == '1'){
                send1();
            }
        }
        sendSync();
    }
}

void RadioSender::setFrequency(RadioSender::Frequency frequency)
{
    m_frequenze = frequency;

    if(m_frequenze == RadioSender::RF433MHz){
        m_pin = 0;
        if(wiringPiSetup() == -1){
            qDebug() << "ERROR: GPIO setup for 433.92 MHz sender failed.";
        }
        qDebug() << "GPIO setup for sender ok.";
        pinMode(m_pin,OUTPUT);
        qDebug() << "sender for GPIO pin" << m_pin << "enabled.";
    }
    if(m_frequenze == RadioSender::RF868MHz){
        qDebug() << "ERROR: 868 MHz Module not connected yet";
    }
}

void RadioSender::setLineCode(RadioSender::LineCode lineCode)
{
    m_lineCode = lineCode;
}

void RadioSender::setPulseLength(int pulseLength)
{
    m_pulseLength = pulseLength;
}

