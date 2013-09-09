#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QDateTime>

#include "radioreciver.h"
#include "wiringPi.h"


// #####################################################################################################
// this class handels the interrupts from the INPUT Pins
class ISRHandler{

private:

    static ISRHandler* s_instance;

    ISRHandler(){
        if(wiringPiSetup() == -1){
            qDebug() << "ERROR: GPIO setup for 433.92 MHz receiver failed.";
        }
        qDebug() << "GPIO setup for receiver ok.";
    }

    // one handler for each pin
    // on pin 0
    static void handleInterrupt0(){
    }
    // on pin 1
    static void handleInterrupt1(){
    }
    // on pin 2
    static void handleInterrupt2(){
        foreach (RadioReciver *receiver, s_receivers){
            if(receiver->getPin() == 2){
                receiver->handleInterrupt();
            }
        }
    }
    // ... and so on -> implement if needed

    static QList<RadioReciver*> s_receivers;

public:
    // create a instance if not allready created
    static ISRHandler* instance(){
        if(!s_instance){
            s_instance = new ISRHandler();
        }
        return s_instance;
    }

    // register a receiver
    void registerReceiver(RadioReciver* receiver){
        bool pinAllreadyRegistred = false;

        foreach (RadioReciver* tmpReceiver, s_receivers) {
            if(tmpReceiver->getPin() == receiver->getPin()){
                pinAllreadyRegistred = true;
            }
        }

        int ok = 0;

        if(!pinAllreadyRegistred){
            pinMode(receiver->getPin(),INPUT);

            // TODO: for more pins I would need more interruptHandlerX...
            switch (receiver->getPin()) {
            case 0:
                ok = wiringPiISR(receiver->getPin(), INT_EDGE_BOTH, &handleInterrupt0);
                break;
            case 1:
                ok = wiringPiISR(receiver->getPin(), INT_EDGE_BOTH, &handleInterrupt1);
                break;
            case 2:
                ok = wiringPiISR(receiver->getPin(), INT_EDGE_BOTH, &handleInterrupt2);
                break;
            default:
                break;
            }
        }
        // check if register the ISR for this pin worked, else the programm exit...
        if(ok != -1){
            s_receivers.append(receiver);
        }
    }

    void unregisterReceiver(RadioReciver* receiver){
        // TODO: delete ISR wiringPi??
        s_receivers.removeAll(receiver);
    }
};
ISRHandler* ISRHandler::s_instance = 0;
QList<RadioReciver*> ISRHandler::s_receivers;


// #####################################################################################################
// The main RadioReceiver class
RadioReciver::RadioReciver(QObject *parent) :
    QObject(parent)
{
    m_enable = false;
    m_pin = -1;
    m_duration = 0;
    m_changeCount = 0;
    m_lastTime = 0;
    m_repeatCount = 0;

    m_thermometer = new RadioThermometer(this);
    m_switch = new RadioSwitch(this);

    for(int i = 0; i < RC_MAX_CHANGES; i++ ){
        m_timings[i] = 0;
    }

    connect(m_thermometer,SIGNAL(temperatureSignalReceived(QByteArray,float,bool)),this,SIGNAL(temperatureSignalReceived(QByteArray,float,bool)));
    connect(m_switch,SIGNAL(switchSignalReceived(QByteArray,char,bool)),this,SIGNAL(switchSignalReceived(QByteArray,char,bool)));
}

void RadioReciver::setFrequency(RadioReciver::Frequency frequency)
{
    m_frequency = frequency;
}

RadioReciver::Frequency RadioReciver::getFrequency() const
{
    return m_frequency;
}

void RadioReciver::setPin(int pin)
{
    m_pin = pin;
}

int RadioReciver::getPin() const
{
    return m_pin;
}


void RadioReciver::handleInterrupt()
{
    if(!m_enable){
        return;
    }

    long currentTime = micros();
    m_duration = currentTime - m_lastTime;

    // filter nois
    if (m_duration > 5000 && m_duration > m_timings[0] - 200 && m_duration < m_timings[0] + 200) {
        m_repeatCount++;
        m_changeCount--;

        if(m_repeatCount == 2) {
            // if we have a regular signal (1 bit sync + 48 bit data)
            if(m_changeCount == RC_MAX_CHANGES){
                // write rawdata to a List and reset values to 0
                QList<int> rawData;
                for(int i = 0; i < RC_MAX_CHANGES; i++ ){
                    rawData.append(m_timings[i]);
                    m_timings[i] = 0;
                }
                detectProtocol(rawData);
            }
            m_repeatCount = 0;
        }
        m_changeCount = 0;

    }else if(m_duration > 5000){
        m_changeCount = 0;
    }
    if (m_changeCount > RC_MAX_CHANGES) {
        m_changeCount = 0;
        m_repeatCount = 0;
    }
    m_timings[m_changeCount++] = m_duration;
    m_lastTime = currentTime;
}

void RadioReciver::detectProtocol(QList<int> rawData)
{
    // check if we have a valid signal, 1 sync + 48 data
    if(rawData.length() != 49){
        return;
    }
    // check plugins
    if(m_thermometer->isValid(rawData)){
        m_thermometer->getTemperature();
    }else
    if(m_switch->isValid(rawData)){
        m_switch->getBinCode();
    }else{
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "|                    GENERIC signal                       |";
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "delay      :" << rawData.first() /31;
        qDebug() << rawData;
    }

}

void RadioReciver::enableReceiver()
{
    // check if we have all needed info...pin, freq...
    if(m_pin == -1){
        qDebug() << "ERROR: pin not set for RadioReceiver";
        return;
    }
    m_enable = true;

    ISRHandler::instance()->registerReceiver(this);
    qDebug() << "receiver for GPIO pin" << m_pin << "enabled.";

}

void RadioReciver::disableReceiver()
{
    m_enable = false;

}

