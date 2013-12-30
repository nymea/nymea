#include "radiojsonplugin.h"
#include <QDebug>
#include <QUuid>

RadioJsonPlugin::RadioJsonPlugin(QObject *parent) :
    JsonPlugin(parent)
{
    m_deviceManager = new DeviceManager(this);
}

QString RadioJsonPlugin::deviceName()
{
    QString deviceName = "radio";
    return deviceName;
}

QStringList RadioJsonPlugin::devicePropertys()
{
    QStringList propertys;
    propertys.append("name");
    propertys.append("uuid");
    propertys.append("channel");
    propertys.append("button");
    propertys.append("buttonState");
    propertys.append("location");
    return propertys;
}

QStringList RadioJsonPlugin::deviceMethods()
{
    QStringList methods;
    methods.append("add");
    methods.append("remove");
    methods.append("edit");
    methods.append("execute");
    methods.append("getAll");
    return methods;
}

QByteArray RadioJsonPlugin::process(const QVariantMap &command, const QVariantMap &parameters)
{
    m_command = command;
    m_parameters = parameters;

    if(!m_command.contains("id")){
        qDebug() << "request contains no id..";
    }

    // check if we have a valid method
    if(deviceMethods().contains(m_command.value("method").toString())){
        // check the methods
        if(m_command.value("method").toString() == "add"){
            qDebug() << "got a ADD DEVICE command";
            if(add()){
                //TODO: return response
            }else{
                // TODO: return error response
            }
        }
        if(m_command.value("method").toString() == "execut"){
            qDebug() << "got a EXECUTE DEVICE command";
            execute();
        }



    }else{
        // TODO: return error respnonse
        return NULL;
    }


}

bool RadioJsonPlugin::add()
{
    QUuid uuid = QUuid::createUuid();
    //qDebug() << uuid;
    return m_deviceManager->saveDevice(m_command.value("deviceType").toString(),uuid,m_parameters);

}

bool RadioJsonPlugin::remove()
{
    if(m_parameters.contains("uuid")){
        QUuid uuid = m_parameters.value("uuid").toUuid();
        return m_deviceManager->deleteDevice(m_command.value("deviceType").toString(),uuid);
    }else{
        return false;
    }
}

bool RadioJsonPlugin::edit()
{

}

bool RadioJsonPlugin::execute()
{
    if(m_parameters.contains("uuid")){
        QUuid uuid = m_parameters.value("uuid").toUuid();

        // load paramters (bincode, linecode, frequency ecc.) and execute


    }else{
        return false;
    }
}

QByteArray RadioJsonPlugin::formatResponse()
{

}

QByteArray RadioJsonPlugin::formatErrorResponse()
{

}
