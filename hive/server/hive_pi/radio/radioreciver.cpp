#include "radioreciver.h"
#include "wiringPi.h"
#include <stdio.h>
#include <QDebug>
#include <QFile>

static bool m_enable = false;
static unsigned int timings[RC_MAX_CHANGES];
static unsigned long m_receivedValue;
static unsigned int m_receivedBitlength;
static unsigned int m_receivedDelay;
static unsigned int m_receiveToleranceThermometer;
static unsigned int m_receiveToleranceRemote;

RadioReciver::RadioReciver(QObject *parent) :
    QObject(parent)
{
    m_receivedBitlength = 0;
    m_receivedDelay = 0;
    m_receivedValue = 0;
    m_receiveToleranceThermometer = 10;
    m_receiveToleranceRemote = 60;
}

void RadioReciver::handleInterrupt()
{
    if(!m_enable){
        return;
    }

    static unsigned int duration;
    static unsigned int changeCount;
    static unsigned long lastTime;
    static unsigned int repeatCount;

    long time = micros();
    duration = time - lastTime;

    // filter nois
    if (duration > 5000 && duration > timings[0] - 200 && duration < timings[0] + 200) {
        repeatCount++;
        changeCount--;
        //qDebug() << "change count" << changeCount;

        if(repeatCount == 2) {
            //qDebug() << "got a pulse with pulselength:" << duration;
            detectProtocol(changeCount);

            for(int i = 0; i < RC_MAX_CHANGES; i++ ){
                timings[i] = 0;
            }

            repeatCount = 0;
        }
        changeCount = 0;

    }else if(duration > 5000){
        changeCount = 0;
    }
    if (changeCount >= RC_MAX_CHANGES) {
        changeCount = 0;
        repeatCount = 0;
    }
    timings[changeCount++] = duration;
    lastTime = time;
}

void RadioReciver::detectProtocol(int signalCount)
{
    qDebug() << "-----------------------------------------------------------";
    //qDebug() << "detect protocoll";
    unsigned long delay = timings[0] / 31;
    //qDebug() << "delay:" << delay;

    // #########################################################################
    //    if(delay > 250 && delay < 260){

    //        QFile file("/root/data.ods");
    //        file.open(QIODevice::WriteOnly | QIODevice::Text);

    //        for(int i = 0; i <= 48; i++){
    //            //file.write << timings[i] << ";\n";
    //        }

    //        file.close();
    //        qDebug() << "-------> got TERMOMETER signal";

    //        QString code = 0;

    //        unsigned long delayTolerance = delay * m_receiveToleranceThermometer * 0.01;
    //        qDebug() << "delay:" << delay << "=" << timings[0] << "/31" << "    delayTolerance = " << delayTolerance << "   , signals = " << changeCount;

    //        return;
    //    }

    // #########################################################################
    if(delay > 310 && delay < 320){

        qDebug() << "-------> got REMOTE Signal";
        qDebug() << "delay:" << delay  << "         bits = " << signalCount-1;

        QString binCode;
        QList <int> rawData;

        // go trough all 48 timings
        for(int i = 1; i <= 48; i+=2 ){
            rawData.append(timings[i]);
            rawData.append(timings[i+1]);
            int div;
            int divNext;

            // if short
            if(timings[i] / delay < 2){
                div = 1;
            }else{
                div = 3;
            }
            // if long
            if(timings[i+1] / delay < 2){
                divNext = 1;
            }else{
                divNext = 3;
            }

            //              _
            // if we have  | |___ = 0 -> in 4 delays
            //                 _
            // if we have  ___| | = 1 -> in 4 delays

            if(div == 1 && divNext == 3){
                binCode.append("0");
            }else if(div == 3 && divNext == 1){
                binCode.append("1");
            }else{
                //qDebug() << "could not read code...error in transmission";
            }
        }


        qDebug() << "raw data:" << rawData;
        qDebug() << "bin CODE:" << binCode;
        qDebug() << "dec CODE:" << binCode;
        qDebug() << "hex CODE:" << binCode;

    }else{
        // #########################################################################
        qDebug() << "-------> got GENERIC Signal";

        unsigned long delayTolerance = delay * m_receiveToleranceRemote * 0.01;
        qDebug() << "delay:" << delay  << "         bits = " << signalCount-1;

        QString binCode;
        QList <int> rawData;

        QFile file("/root/rc433_log_data.ods");
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream out(&file);

        for(int i = 0; i < signalCount; i+=1 ){
            out << timings[i] << ",";
        }
        out << ";\n";
        file.close();

        // go trough all 48 timings
        for(int i = 0; i <= 48; i+=1 ){
            rawData.append(timings[i]);
            rawData.append(timings[i+1]);
        }

        qDebug() << "raw data:" << rawData;

    }
}

void RadioReciver::enableReceiver()
{
    m_enable = true;
}

void RadioReciver::disableReceiver()
{
    m_enable = false;
}

void RadioReciver::setFrequence(RadioReciver::Frequenze frequenze)
{
    if(frequenze == RadioReciver::RC433MHz){
        if(wiringPiSetup() == -1){
            qDebug() << "ERROR: GPIO setup failed.";
        }
        qDebug() << "GPIO setup ok.";
        pinMode(2,INPUT);
        wiringPiISR(2, INT_EDGE_BOTH, &handleInterrupt);
    }
    if(frequenze == RadioReciver::RC868MHz){
        qDebug() << "ERROR: 868 MHz Module not connected yet";
    }


}
