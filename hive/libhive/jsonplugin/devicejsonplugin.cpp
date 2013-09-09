#include "devicejsonplugin.h"
#include <QDebug>
#include <QUuid>
#include <QSettings>

DeviceJsonPlugin::DeviceJsonPlugin(QObject *parent) :
    JsonPlugin(parent)
{
    m_deviceManager = new DeviceManager(this);
}

QString DeviceJsonPlugin::deviceName()
{
    QString deviceName = "device";
    return deviceName;

}

QByteArray DeviceJsonPlugin::process(const QVariantMap &command, const QVariantMap &parameters)
{

    m_command = command;
    m_parameters = parameters;

    // check if we have a id
    if(!m_command.contains("id")){
        qDebug() << "request contains no id..";
    }

    // check the methods
    if(m_command.value("method").toString() == "add"){
        qDebug() << "got a ADD DEVICE command";
        add();
    }

    return 0;
}

bool DeviceJsonPlugin::add()
{
//    QUuid uuid = QUuid::createUuid();
//    //qDebug() << uuid;
//    if(parameters.value("deviceType").toString() == "actor"){
//        QSettings settings("hive");
//        settings.beginGroup(uuid.toString());
//        QMapIterator<QString, QVariant> i(parameters);
//        while(i.hasNext()){
//            i.next();
//            qDebug() << i.key() << "=" << i.value().toString();
//            settings.setValue(i.key(),i.value());
//        }
//        settings.endGroup();
//    }
}

bool DeviceJsonPlugin::remove()
{
    
}

QByteArray DeviceJsonPlugin::formatResponse()
{

}

QByteArray DeviceJsonPlugin::formatErrorResponse()
{

}
