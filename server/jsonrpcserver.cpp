#include "jsonrpcserver.h"

#include "tcpserver.h"

#include "hivecore.h"
#include "devicemanager.h"
#include "deviceclass.h"

#include <QJsonDocument>
#include <QStringList>

JsonRPCServer::JsonRPCServer(QObject *parent):
    QObject(parent),
    m_tcpServer(new TcpServer(this))
{
    connect(m_tcpServer, &TcpServer::jsonDataAvailable, this, &JsonRPCServer::processData);
    m_tcpServer->startServer();
}

void JsonRPCServer::processData(int clientId, const QByteArray &jsonData)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << jsonData << ":" << error.errorString();
    }
    qDebug() << "-------------------------\n" << jsonDoc.toJson();

    QVariantMap message = jsonDoc.toVariant().toMap();

    bool success;
    int commandId = message.value("id").toInt(&success);
    if (!success) {
        qWarning() << "Error parsing command. Missing \"id\":" << jsonData;
        return;
    }

    QStringList commandList = message.value("method").toString().split('.');
    if (commandList.count() != 2) {
        qWarning() << "Error parsing method.\nGot:" << message.value("method").toString() << "\nExpected: \"Namespace.method\"";
        return;
    }

    QString targetNamspace = commandList.first();
    QString method = commandList.last();
    QVariantMap params = message.value("params").toMap();

    qDebug() << "got:" << targetNamspace << method << params;
    emit commandReceived(targetNamspace, method, params);

    if (targetNamspace == "Devices") {
        if (method == "GetSupportedDevices") {
            qDebug() << "getSupportedDevices";
            QVariantMap params;
            QVariantList supportedDeviceList;
            foreach (const DeviceClass &deviceClass, HiveCore::instance()->deviceManager()->supportedDevices()) {
                supportedDeviceList.append(packDeviceClass(deviceClass));
            }
            params.insert("deviceClasses", supportedDeviceList);
            sendResponse(clientId, commandId, params);
        }
    } else {
        qDebug() << "got unknown namespace" << targetNamspace;
    }

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

QVariantMap JsonRPCServer::packDeviceClass(const DeviceClass &deviceClass)
{
    QVariantMap variant;
    variant.insert("name", deviceClass.name());
    variant.insert("id", deviceClass.id());
    QVariantList triggerTypes;
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        QVariantMap triggerMap;
        triggerMap.insert("id", triggerType.id().toString());
        triggerMap.insert("name", triggerType.name());
        triggerMap.insert("params", triggerType.parameters());

        triggerTypes.append(triggerMap);
    }
    variant.insert("triggers", triggerTypes);
    return variant;
}

void JsonRPCServer::sendResponse(int clientId, int commandId, const QVariantMap &params)
{
    QVariantMap rsp;
    rsp.insert("id", commandId);
    rsp.insert("params", params);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(rsp);
    m_tcpServer->sendResponse(clientId, jsonDoc.toJson());
}


