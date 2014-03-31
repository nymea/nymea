/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
    \page meisteranker.html
    \title Meister Anker

    \ingroup plugins
    \ingroup rf433

    This plugin allows to receive thermometer signals from Meister Anker
    devices.

    Following devices are supported:

    \chapter Supported devices
        \section1 Thermometer
            \table
            \header
                \li Model
                \li Device Type
            \row
                \li
                \li
            \endtable
  */

#include "devicepluginmeisteranker.h"

#include "device.h"
#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>
#include <QStringList>

QUuid thermometer = QUuid("e37e9f34-95b9-4a22-ae4f-e8b874eec871");

DevicePluginMeisterAnker::DevicePluginMeisterAnker()
{
}

QList<DeviceClass> DevicePluginMeisterAnker::supportedDevices() const
{
    QList<DeviceClass> ret;

    // Thermometer
    DeviceClass deviceClassMeisterAnkerThermometer(pluginId(), thermometer);
    deviceClassMeisterAnkerThermometer.setName("Meister Anker Thermometer");
    
    QVariantList thermometerParams;
    QVariantMap idParam;

    // =======================================
    // id -> first 8 bits of codeword
    idParam.insert("name", "id");
    idParam.insert("type", "string");
    thermometerParams.append(idParam);

    deviceClassMeisterAnkerThermometer.setParams(thermometerParams);

    QList<StateType> thermometerStates;

    StateType tempState("a9849491-25f4-43a3-a6fe-3bfce43d6332");
    tempState.setName("Temperature");
    tempState.setType(QVariant::Double);
    thermometerStates.append(tempState);

    StateType batteryState("ebf951ba-75ca-47ac-ba3c-ea9ec1e7bbd1");
    batteryState.setName("Battery");
    batteryState.setType(QVariant::Bool);
    thermometerStates.append(batteryState);

    deviceClassMeisterAnkerThermometer.setStates(thermometerStates);

    QList<EventType> thermometerEvents;
    
    QVariantList paramsThermometer;
    QVariantMap paramThermometer;
    paramThermometer.insert("name", "temperature");
    paramThermometer.insert("type", "double");
    paramsThermometer.append(paramThermometer);

    EventType temperatureEvent(QUuid("174ab4d5-2ef0-491b-a55b-c895cedff80e"));
    temperatureEvent.setName("temperature");
    temperatureEvent.setParameters(paramsThermometer);
    thermometerEvents.append(temperatureEvent);

    QVariantList paramsThermometerBat;
    QVariantMap paramThermometerBat;
    paramThermometerBat.insert("name", "batteryStatus");
    paramThermometerBat.insert("type", "bool");
    paramsThermometerBat.append(paramThermometerBat);

    EventType batteryStatusEvent(QUuid("c376b532-993f-41c7-acc7-02b409136d32"));
    batteryStatusEvent.setName("batteryStatus");
    batteryStatusEvent.setParameters(paramsThermometerBat);
    thermometerEvents.append(batteryStatusEvent);

    // TODO: lock if we need a sync event
    //    EventType syncEvent(QUuid("174ab4d5-2ef0-491b-a55b-c895cedff80e"));
    //    temperatureEvent.setName("sync");
    //    temperatureEvent.setParameters(paramsThermometer);
    //    thermometerEvents.append(temperatureEvent);


    deviceClassMeisterAnkerThermometer.setEvents(thermometerEvents);
    ret.append(deviceClassMeisterAnkerThermometer);

    return ret;
}

DeviceManager::HardwareResources DevicePluginMeisterAnker::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

QString DevicePluginMeisterAnker::pluginName() const
{
    return "Meister Anker";
}

QUuid DevicePluginMeisterAnker::pluginId() const
{
    return QUuid("993a7c86-e4b9-44aa-b61e-1f7165df1348");
}

void DevicePluginMeisterAnker::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device)
    Q_UNUSED(action)
}

void DevicePluginMeisterAnker::radioData(QList<int> rawData)
{    
    // filter right here a wrong signal length
    if(rawData.length() != 49){
        return;
    }

    QByteArray binCode;
    int delay = rawData.first()/31;

    // parse rawdate to binary code
    if(delay > 240 && delay < 260){

        /*    __
         *   |  |________         = 0     1100000000
         *    __
         *   |  |________________ = 1     110000000000000000
         */

        for(int i = 1; i <= 48; i+=2 ){
            if(rawData.at(i) < 1000 && rawData.at(i+1) < 3000 && rawData.at(i+1) > 1000){
                binCode.append('0');
            }else if(rawData.at(i) < 1000 && rawData.at(i+1) > 3000){
                binCode.append('1');
            }else{
                return;
            }
        }
    }else{
        return;
    }

    //qDebug() << "bin code" << binCode;

    // =======================================
    // {     ID    },{   temp    },{Batt},{,temp}
    // "XXXX","XXXX","XXXX","XXXX","XXXX","XXXX",

    QString idCode = QString(binCode.left(8));
    QByteArray temperatureBin = binCode.mid(8,8);
    QByteArray batteryBin = binCode.mid(16,4);
    QByteArray temperatureTenthBin = binCode.right(4);

    qDebug() << "id:" << idCode;
    qDebug() << "battery" << batteryBin;

    // check if we have a sync signal (id = 11111111)
    if(idCode == "11111111"){
        qDebug() << "temperatursensor sync signal";
        return;
    }

    // check if the battery is low
    bool batteryStatus;
    if(batteryBin.toInt(0,2) == 0){
        batteryStatus = true;
    }else{
        batteryStatus = false;
    }

    // check sign of temperature -> if first bit of temperature byte is 1 -> temp is negativ
    int sign = 0;
    if(temperatureBin.left(1).toInt() == 1){
        sign = -1;
    }else{
        sign = 1;
    }

    // calc temperature
    float temperature = sign*(temperatureBin.right(7).toInt(0,2) + (float)temperatureTenthBin.toInt(0,2)/10);

    // TODO: check if it is the same temperature than the last time
    // QString timeStamp = QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm:ss");

}
