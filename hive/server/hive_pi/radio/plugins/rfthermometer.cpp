#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QDateTime>

#include "rfthermometer.h"

RFThermometer::RFThermometer(QObject *parent) :
    RadioPlugin(parent)
{
    m_lastTemperature = -1111;
    m_delay = 0;
    m_binCode = 0;

}

QByteArray RFThermometer::getBinCode()
{
    if(m_binCode.isEmpty()){
        return NULL;
    }else{
        return m_binCode;
    }
}


bool RFThermometer::isValid(QList<int> rawData)
{
    m_delay = rawData.first()/31;
    QByteArray binCode;

    if(m_delay > 250 && m_delay < 260){

        //  __
        // |  |________         = 0     1100000000
        //  __
        // |  |________________ = 1     110000000000000000

        for(int i = 1; i <= 48; i+=2 ){
            if(rawData.at(i) < 1000 && rawData.at(i+1) < 3000 && rawData.at(i+1) > 1000){
                binCode.append('0');
            }else if(rawData.at(i) < 1000 && rawData.at(i+1) > 3000){
                binCode.append('1');
            }else{
                return false;
            }
        }
        m_binCode = binCode;
        return true;
    }else{
        m_binCode = 0;
        return false;
    }
}

float RFThermometer::getTemperature()
{
    // {     ID    },{-+}{ temp,   },{Batt},{,temp}
    // "XXXX","XXXX","X  XXX","XXXX","XXXX","XXXX",

    QByteArray binCode = m_binCode;

    QList<QByteArray> byteList;
    for(int i = 4; i <= 24; i+=4){
        byteList.append(binCode.left(4));
        binCode = binCode.right(binCode.length() -4);
    }

    QByteArray temperatureBin(byteList.at(2) + byteList.at(3));
    QByteArray batteryBin(byteList.at(4));
    QByteArray temperatureTenthBin(byteList.at(5));


    // check sign of temperature -> if first bit of temperature byte is 1 -> temp is negativ
    int sign = 0;
    if(temperatureBin.left(1).toInt() == 1){
        sign = -1;
    }else{
        sign = 1;
    }

    //qDebug() << temperatureBin << "=" << temperatureBin.left(1) << temperatureBin.right(7) << temperatureBin.right(7).toInt(0,2) << "," << temperatureTenthBin.toInt(0,2) ;

    // calc temperature
    float temperature = sign*(temperatureBin.right(7).toInt(0,2) + (float)temperatureTenthBin.toInt(0,2)/10);

    // check if the battery is low
    bool batteryStatus;
    if(batteryBin.toInt(0,2) == 0){
        batteryStatus = true;
    }else{
        batteryStatus = false;
    }

    if(temperature == m_lastTemperature){
        return temperature;
    }else{

        m_lastTemperature = temperature;
        QString timeStamp = QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm:ss");;
        //qDebug() << timeStamp;

        QFile file("/root/temperature_log.ods");
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream out(&file);

        QByteArray id = byteList.at(0)+byteList.at(1);

        out << timeStamp << "," << temperature << "," << batteryStatus << "\n";
        file.close();
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "|                  THERMOMETER signal                     |";
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "delay      :" << m_delay;
        qDebug() << "bin CODE   :" << m_binCode;
        qDebug() << byteList;
        qDebug() << timeStamp << " ID:" << id << " Temperature:" << temperature << " Battery OK: " << batteryStatus;

        //emit temperatureSignalReceived(id,temperature,batteryStatus);
        return temperature;
    }
}
