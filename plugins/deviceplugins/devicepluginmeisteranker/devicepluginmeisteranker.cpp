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

void DevicePluginMeisterAnker::init()
{
    connect(deviceManager()->radio433(), &Radio433::dataReceived, this, &DevicePluginMeisterAnker::dataReceived);
}

QList<DeviceClass> DevicePluginMeisterAnker::supportedDevices() const
{
    QList<DeviceClass> ret;

    // Thermometer
    DeviceClass deviceClassMeisterAnkerThermometer(pluginId(), thermometer);
    deviceClassMeisterAnkerThermometer.setName("Meister Anker Thermometer");
    
    QVariantList thermometerParams;
    QVariantMap idParam;
    // id -> first 8 bits of codeword
    idParam.insert("name", "id");
    idParam.insert("type", "string");
    thermometerParams.append(idParam);

    deviceClassMeisterAnkerThermometer.setParams(thermometerParams);
    
    QList<TriggerType> thermometerTriggers;
    
    QVariantList paramsThermometer;

    QVariantMap paramThermometer;
    paramThermometer.insert("name", "temperature");
    paramThermometer.insert("type", "double");
    paramsThermometer.append(paramThermometer);

    QVariantMap paramThermometerBat;
    paramThermometerBat.insert("name", "batterystatus");
    paramThermometerBat.insert("type", "bool");
    paramsThermometer.append(paramThermometerBat);

    TriggerType temperatureTrigger(QUuid("174ab4d5-2ef0-491b-a55b-c895cedff80e"));
    temperatureTrigger.setName("temperature");
    temperatureTrigger.setParameters(paramsThermometer);
    thermometerTriggers.append(temperatureTrigger);

    // TODO: lock if we need a sync trigger
    //    TriggerType syncTrigger(QUuid("174ab4d5-2ef0-491b-a55b-c895cedff80e"));
    //    temperatureTrigger.setName("sync");
    //    temperatureTrigger.setParameters(paramsThermometer);
    //    thermometerTriggers.append(temperatureTrigger);


    deviceClassMeisterAnkerThermometer.setTriggers(thermometerTriggers);
    ret.append(deviceClassMeisterAnkerThermometer);

    return ret;
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

}

void DevicePluginMeisterAnker::dataReceived(QList<int> rawData)
{    
    // filter right here a wrong signal length
    if(rawData.length() != 49){
        return;
    }
    
    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(thermometer);
    if(deviceList.isEmpty()){
        return;
    }

    int delay = rawData.first()/31;
    QByteArray binCode;
    
    if(delay > 250 && delay < 260){

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
                return;
            }
        }
    }else{
        return;
    }

    // decode the signal
    QList<QByteArray> byteList;
    for(int i = 4; i <= 24; i+=4){
        byteList.append(binCode.left(4));
        binCode = binCode.right(binCode.length() -4);
    }

    QByteArray temperatureBin(byteList.at(2) + byteList.at(3));
    QByteArray batteryBin(byteList.at(4));
    QByteArray temperatureTenthBin(byteList.at(5));

    QByteArray idCode = binCode.left(8);

    // check if we have a sync signal (id = 11111111)
    if(idCode.contains("11111111")){
        qDebug() << "temperatursensor sync signal";
    }


}
