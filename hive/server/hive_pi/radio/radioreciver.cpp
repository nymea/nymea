#include "radioreciver.h"
#include "wiringPi.h"
#include <stdio.h>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QDateTime>

static float lastTemperature = 0;


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

    for(int i = 0; i < RC_MAX_CHANGES; i++ ){
        m_timings[i] = 0;
    }

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
    qDebug() << "===========================================================================";
    qDebug() <<"VALID SIGNAL (48 bit)" << " -->  pulse width =" << rawData.first()/31;
    qDebug() << rawData;

//    int pulseWidth = rawData.first()/31;

//    QList<int> rawDataKGV;
//    foreach (int timing, rawData){
//        int kgv = ((float)timing/pulseWidth)+0.5;
//        rawDataKGV.append(kgv);
//    }
//    qDebug() << "-------= " << rawDataKGV;

}


//void RadioReciver::detectProtocol(int signalCount)
//{
//    if(signalCount < 49){
//        //qDebug() << "ERROR: got a signal with just" << signalCount << "signals";
//        return;
//    }
//    qDebug() << "===========================================================================";
//    QList <int> rawData;

//    // go trough all 49 timings
//    for(int i = 0; i <= 48; i+=1 ){
//        rawData.append(timings[i]);
//    }
//    qDebug() << "raw data:" << rawData;

//    unsigned long delay = timings[0] / 31;

//    // #########################################################################
//    if(delay > 250 && delay < 260){
//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << "            seems to be a TERMOMETER signal";
//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << "delay      :" << delay;
//        qDebug() << "bits       :" << signalCount-1;

//        QByteArray binCode;

//        //  __
//        // |  |________         = 0     1100000000
//        //  __
//        // |  |________________ = 1     110000000000000000

//        for(int i = 1; i <= 48; i+=2 ){
//            if(timings[i] < 1000 && timings[i+1] < 3000 && timings[i+1] > 1000){
//                binCode.append('0');
//            }else if(timings[i] < 1000 && timings[i+1] > 3000){
//                binCode.append('1');
//            }else{
//                qDebug() << "ERROR: could not read code...error in transmission";
//                return;
//            }
//        }

//        qDebug() << "bin CODE   :" << binCode;
//        parseTemperature(binCode);

//        return;
//    }

//    // #########################################################################
//    if(delay > 310 && delay < 340){

//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << "            seems to be a REMOTE signal";
//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << "delay      :" << delay;
//        qDebug() << "bits       :" << signalCount-1;

//        QByteArray binCode;
//        QList <int> rawData;

//        // go trough all 48 timings
//        for(int i = 1; i <= 48; i+=2 ){
//            rawData.append(timings[i]);
//            rawData.append(timings[i+1]);
//            int div;
//            int divNext;

//            // if short
//            if(timings[i] / delay < 2){
//                div = 1;
//            }else{
//                div = 3;
//            }
//            // if long
//            if(timings[i+1] / delay < 2){
//                divNext = 1;
//            }else{
//                divNext = 3;
//            }

//            //              _
//            // if we have  | |___ = 0 -> in 4 delays => 1000
//            //                 _
//            // if we have  ___| | = 1 -> in 4 delays => 0001

//            if(div == 1 && divNext == 3){
//                binCode.append("0");
//            }else if(div == 3 && divNext == 1){
//                binCode.append("1");
//            }else{
//                qDebug() << "ERROR: could not read code...error in transmission";
//                return;
//            }
//        }

//        //qDebug() << "raw data:" << rawData;
//        qDebug() << "bin CODE   :" << binCode;
//        QStringList byteList;
//        for(int i = 4; i <= 24; i+=4){
//            byteList.append(binCode.left(4));
//            binCode = binCode.right(binCode.length() -4);
//        }
//        qDebug() << byteList;


//    }else{
//        // #########################################################################
//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << "            seems to be a GENERIC signal";
//        qDebug() << "-----------------------------------------------------------";
//        qDebug() << "delay      :" << delay;
//        qDebug() << "bits       : " << signalCount-1;

//        QList <int> rawData;

//        QFile file("/root/rc433_log_data.ods");
//        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
//        QTextStream out(&file);

//        for(int i = 0; i < signalCount; i+=1 ){
//            out << timings[i] << ",";
//            rawData.append(timings[i]);
//        }
//        out << ";\n";
//        file.close();
//        qDebug() << "raw data:" << rawData;


//    }
//}

//float RadioReciver::parseTemperature(QByteArray codeBin)
//{
//    // {     ID    },{-+}{ temp,   },{Batt},{,temp}
//    // "XXXX","XXXX","X  XXX","XXXX","XXXX","XXXX",

//    QList<QByteArray> byteList;
//    for(int i = 4; i <= 24; i+=4){
//        byteList.append(codeBin.left(4));
//        codeBin = codeBin.right(codeBin.length() -4);
//    }
//    qDebug() << byteList;

//    QByteArray temperatureBin(byteList.at(2) + byteList.at(3));
//    QByteArray batteryBin(byteList.at(4));
//    QByteArray temperatureTenthBin(byteList.at(5));


//    // check sign of temperature -> if first bit of temperature byte is 1 -> temp is negativ
//    int sign = 0;
//    if(temperatureBin.left(1).toInt() == 1){
//        sign = -1;
//    }else{
//        sign = 1;
//    }

//    //qDebug() << temperatureBin << "=" << temperatureBin.left(1) << temperatureBin.right(7) << temperatureBin.right(7).toInt(0,2) << "," << temperatureTenthBin.toInt(0,2) ;

//    // calc temperature
//    float temperature = sign*(temperatureBin.right(7).toInt(0,2) + (float)temperatureTenthBin.toInt(0,2)/10);

//    // check if the battery is low
//    QString batteryStatus;
//    if(batteryBin.toInt(0,2) == 0){
//        batteryStatus = "ok";
//    }else{
//        batteryStatus = "low";
//    }

//    qDebug() << "Temperature:" << temperature << "Battery: " << batteryStatus;

//    if(temperature == lastTemperature){
//        return temperature;
//    }else{
//        lastTemperature = temperature;
//        QString timeStamp = QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm");;
//        qDebug() << timeStamp;

//        QFile file("/root/temperature_log.ods");
//        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
//        QTextStream out(&file);

//        out << timeStamp << "," << temperature << "," << batteryStatus << "\n";
//        file.close();
//    }

//    return temperature;
//}

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

