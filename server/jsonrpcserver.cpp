#include "jsonrpcserver.h"

#include "tcpserver.h"

#include <QJsonDocument>

JsonRPCServer::JsonRPCServer(QObject *parent):
    m_tcpServer(new TcpServer(this))
{
    connect(m_tcpServer, &TcpServer::jsonDataAvailable, this, &JsonRPCServer::processData);
    m_tcpServer->startServer();
}

void JsonRPCServer::processData(const QByteArray &jsonData)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << jsonData << ":" << error.errorString();
    }
    qDebug() << "-------------------------\n" << jsonDoc.toJson();

    QVariantMap command = jsonDoc.toVariant().toMap();
    QVariantMap params = jsonDoc.toVariant().toMap().value("params").toMap();

    //DeviceJsonPlugin plugin;
    // {<device>: "name", <method>: "doBla", <id>: "int", <command> { <name>: "name", ...  }}

//    if(command.value("device").toString() == m_device->deviceName()){
//        return m_device->process(command,params);
//    }
//    if(command.value("device").toString() == m_radio->deviceName()){
//        return m_radio->process(command,params);
//    }else{
//        return NULL;
//    }

}


