#include "jsonhandler.h"

#include <QJsonDocument>
#include <QVariantMap>
#include <QDebug>

JsonHandler::JsonHandler(QObject *parent) :
    QObject(parent)
{
    m_device = new DeviceJsonPlugin(this);
    m_radio = new RadioJsonPlugin(this);
}



QByteArray JsonHandler::process(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << data << ":" << error.errorString();
    }
    qDebug() << "-------------------------\n" << jsonDoc.toJson();

    QVariantMap command = jsonDoc.toVariant().toMap();
    QVariantMap params = jsonDoc.toVariant().toMap().value("params").toMap();

    //DeviceJsonPlugin plugin;
    // {<device>: "name", <method>: "doBla", <id>: "int", <command> { <name>: "name", ...  }}

    if(command.value("device").toString() == m_device->deviceName()){
        return m_device->process(command,params);
    }
    if(command.value("device").toString() == m_radio->deviceName()){
        return m_radio->process(command,params);
    }else{
        return NULL;
    }


}

QByteArray JsonHandler::formatResponse(const QVariantMap &command, const QVariantMap &responseParams)
{
    QVariantMap responseMap;
    responseMap.insert("id", command.value("id"));
    responseMap.insert("success", true);
    responseMap.insert("params", responseParams);
    QByteArray response = QJsonDocument::fromVariant(responseMap).toBinaryData();
    response.append(QString('\n'));
    qDebug() << "ERROR response: " << response;
    return response;
}

QByteArray JsonHandler::formatErrorResponse(const QVariantMap &command, const QString &error)
{
    QVariantMap responseMap;
    responseMap.insert("id", command.value("id"));
    responseMap.insert("success", false);
    responseMap.insert("error", error);
    QByteArray response = QJsonDocument::fromVariant(responseMap).toBinaryData();
    response.append(QString('\n'));
    qDebug() << "ERROR response: " << response;
    return response;
}
