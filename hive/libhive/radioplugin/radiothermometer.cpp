#include "radiothermometer.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

RadioThermometer::RadioThermometer(QObject *parent) :
    RadioPlugin(parent)
{
    m_lastTemperature = -1111;
    m_delay = 0;
    m_binCode = 0;
}

QByteArray RadioThermometer::getBinCode()
{
    if(m_binCode.isEmpty()){
        return NULL;
    }else{
        return m_binCode;
    }
}

bool RadioThermometer::isValid(QList<int> rawData)
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
        m_binCode.clear();
        return false;
    }
}

float RadioThermometer::getTemperature()
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

    QByteArray id = byteList.at(0)+byteList.at(1);

    // check if we have a sync signal (id = 11111111)
    if(id.contains("11111111")){
        qDebug() << "temperatursensor sync signal";
        return 0;
    }

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
        qDebug() << "received same temperature from " << QString(id) << " - " << QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm:ss");
        return temperature;
    }else{

        m_lastTemperature = temperature;
        QString timeStamp = QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm:ss");
        //qDebug() << timeStamp;

        QFile file("/root/temperature_log.ods");
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream out(&file);

        out << id << "," << timeStamp << ","  << batteryStatus << "," << temperature << "," << "\n";
        file.close();

        qDebug() << "-----------------------------------------------------------";
        qDebug() << "|                  THERMOMETER signal                     |";
        qDebug() << "-----------------------------------------------------------";
        qDebug() << "delay      :" << m_delay;
        qDebug() << "bin CODE   :" << m_binCode;
        qDebug() << byteList;
        qDebug() << timeStamp << " ID:" << id << " Temperature:" << temperature << " Battery OK: " << batteryStatus;

        emit temperatureSignalReceived(id,temperature,batteryStatus);
        return temperature;
    }
}
