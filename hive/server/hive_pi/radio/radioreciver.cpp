#include "radioreciver.h"
#include "wiringPi.h"
#include <stdio.h>
#include <QDebug>
#include <QFile>
#include <QStringList>

static bool m_enable = false;
static unsigned int timings[RC_MAX_CHANGES];


RadioReciver::RadioReciver(QObject *parent) :
    QObject(parent)
{

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

        if(repeatCount == 2) {
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
    qDebug() << "===========================================================================";
    if(signalCount < 49){
        qDebug() << "ERROR: got a signal with just" << signalCount << "signals";
        return;
    }
    QList <int> rawData;

    // go trough all 49 timings
    for(int i = 0; i <= 48; i+=1 ){
        rawData.append(timings[i]);
    }
    qDebug() << "raw data:" << rawData;

    unsigned long delay = timings[0] / 31;

    // #########################################################################
        if(delay > 250 && delay < 260){
            qDebug() << "-----------------------------------------------------------";
            qDebug() << "            seems to be a TERMOMETER signal";
            qDebug() << "-----------------------------------------------------------";
            qDebug() << "delay      :" << delay;
            qDebug() << "bits       :" << signalCount-1;

            QByteArray binCode;

            for(int i = 1; i <= 48; i+=2 ){
                if(timings[i] < 1000 && timings[i+1] < 3000 && timings[i+1] > 1000){
                    binCode.append('0');
                }else if(timings[i] < 1000 && timings[i+1] > 3000){
                    binCode.append('1');
                }else{
                    qDebug() << "ERROR: could not read code...error in transmission";
                    return;
                }
            }

            qDebug() << "bin CODE   :" << binCode;
            parseTemperature(binCode);

            return;
        }

    // #########################################################################
    if(delay > 310 && delay < 320){

        qDebug() << "-----------------------------------------------------------";
        qDebug() << "            seems to be a REMOTE signal";
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "delay      :" << delay;
        qDebug() << "bits       :" << signalCount-1;

        QByteArray binCode;
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
                qDebug() << "ERROR: could not read code...error in transmission";
                return;
            }
        }

        //qDebug() << "raw data:" << rawData;
        qDebug() << "bin CODE   :" << binCode;
        QStringList byteList;
        for(int i = 4; i <= 24; i+=4){
            byteList.append(binCode.left(4));
            binCode = binCode.right(binCode.length() -4);
        }
        qDebug() << byteList;


    }else{
        // #########################################################################
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "            seems to be a GENERIC signal";
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "delay      :" << delay;
        qDebug() << "bits       : " << signalCount-1;

        QList <int> rawData;

        QFile file("/root/rc433_log_data.ods");
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream out(&file);

        for(int i = 0; i < signalCount; i+=1 ){
            out << timings[i] << ",";
            rawData.append(timings[i]);
        }
        out << ";\n";
        file.close();
        qDebug() << "raw data:" << rawData;


    }
}

float RadioReciver::parseTemperature(QByteArray codeBin)
{
    // {     ID    },{-+}{ temp,   },{Batt},{,temp}
    // "XXXX","XXXX","X  XXX","XXXX","XXXX","XXXX",

    QList<QByteArray> byteList;
    for(int i = 4; i <= 24; i+=4){
        byteList.append(codeBin.left(4));
        codeBin = codeBin.right(codeBin.length() -4);
    }
    qDebug() << byteList;

    QByteArray temperatureBin(byteList.at(2) + byteList.at(3));
    QByteArray temperatureTenthBin(byteList.at(5));

    // get sign of temperature -> if first bit of temperature byte is 1 -> temp is negativ
    int sign = 0;
    if(temperatureBin.left(1).toInt() == 1){
        sign = -1;
    }else{
        sign = 1;
    }

    qDebug() << temperatureBin << "=" << temperatureBin.left(1) << temperatureBin.right(7) << temperatureBin.right(7).toInt(0,2) << "," << temperatureTenthBin.toInt(0,2) ;

    float temperature = sign*(temperatureBin.right(7).toInt(0,2) + (float)temperatureTenthBin.toInt(0,2)/10);

    qDebug() << "Temperature:" << temperature;
    return temperature;
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
