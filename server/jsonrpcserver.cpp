#include "jsonrpcserver.h"

#include "tcpserver.h"

#include "hivecore.h"
#include "devicemanager.h"
#include "deviceclass.h"
#include "device.h"
#include "rule.h"
#include "ruleengine.h"

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

    QString targetNamespace = commandList.first();
    QString method = commandList.last();
    QVariantMap params = message.value("params").toMap();

    qDebug() << "got:" << targetNamespace << method << params;
    emit commandReceived(targetNamespace, method, params);

    if (targetNamespace == "Devices") {
        if (method == "GetSupportedDevices") {
            QVariantMap params;
            QVariantList supportedDeviceList;
            foreach (const DeviceClass &deviceClass, HiveCore::instance()->deviceManager()->supportedDevices()) {
                supportedDeviceList.append(packDeviceClass(deviceClass));
            }
            params.insert("deviceClasses", supportedDeviceList);
            sendResponse(clientId, commandId, params);
        } else if (method == "AddConfiguredDevice") {
            QUuid deviceClass = params.value("deviceClass").toUuid();
            QVariantMap deviceParams = params.value("deviceParams").toMap();
            DeviceManager::DeviceError status = HiveCore::instance()->deviceManager()->addConfiguredDevice(deviceClass, deviceParams);
            switch(status) {
            case DeviceManager::DeviceErrorNoError:
                sendResponse(clientId, commandId);
                break;
            case DeviceManager::DeviceErrorDeviceClassNotFound:
                sendErrorResponse(clientId, commandId, "Error creating device. Device class not found.");
                break;
            case DeviceManager::DeviceErrorMissingParameter:
                sendErrorResponse(clientId, commandId, "Error creating device. Missing parameter.");
                break;
            }
        } else if (method == "GetConfiguredDevices") {
            QVariantMap rspParams;
            QVariantList configuredDeviceList;
            foreach (Device *device, HiveCore::instance()->deviceManager()->configuredDevices()) {
                configuredDeviceList.append(packDevice(device));
            }
            rspParams.insert("devices", configuredDeviceList);
            sendResponse(clientId, commandId, rspParams);
        } else {
            sendErrorResponse(clientId, commandId, "No such method");
        }
    } else if (targetNamespace == "Rules") {
        handleRulesMessage(clientId, commandId, method, params);
    } else if (targetNamespace == "Actions") {
        handleActionMessage(clientId, commandId, method, params);
    } else {
        qDebug() << "got unknown namespace" << targetNamespace;
    }
}

void JsonRPCServer::handleRulesMessage(int clientId, int commandId, const QString &method, const QVariantMap &params)
{
    if (method == "GetRules") {
        QVariantList rulesList;
        foreach (const Rule &rule, HiveCore::instance()->ruleEngine()->rules()) {
            QVariantMap ruleMap;
            ruleMap.insert("id", rule.id());
            ruleMap.insert("triggerId", rule.triggerId());
            ruleMap.insert("action", packAction(rule.action()));
            rulesList.append(ruleMap);
        }
        QVariantMap rspParams;
        rspParams.insert("rules", rulesList);
        sendResponse(clientId, commandId, rspParams);
    } else if (method == "AddRule") {
        QUuid triggerId = params.value("triggerId").toUuid();
        QUuid actionId = params.value("actionId").toUuid();
        switch(HiveCore::instance()->ruleEngine()->addRule(triggerId, actionId)) {
        case RuleEngine::RuleErrorNoError:
            sendResponse(clientId, commandId);
            break;
        case RuleEngine::RuleErrorNoSuchTrigger:
            sendErrorResponse(clientId, commandId, "No such trigger");
            break;
        case RuleEngine::RuleErrorNoSuchAction:
            sendErrorResponse(clientId, commandId, "No such action");
            break;
        }
    }
}

void JsonRPCServer::handleActionMessage(int clientId, int commandId, const QString &method, const QVariantMap &params)
{
    if (method == "ExecuteAction") {
        QUuid deviceId = QUuid(params.value("deviceId").toUuid());
        QVariantList actionParams = params.value("params").toList();

        Action action(QUuid::createUuid(), deviceId);
        action.setParams(actionParams);

        HiveCore::instance()->deviceManager()->executeAction(action);

    }
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
    QVariantList actionTypes;
    qDebug() << "*******************" << deviceClass.actions().count();
    foreach (const ActionType &actionType, deviceClass.actions()) {
        QVariantMap actionMap;
        actionMap.insert("id", actionType.id().toString());
        actionMap.insert("name", actionType.name());
        actionMap.insert("params", actionType.parameters());

        actionTypes.append(actionMap);
    }
    variant.insert("params", deviceClass.params());
    variant.insert("triggers", triggerTypes);
    variant.insert("actions", actionTypes);
    return variant;
}

QVariantMap JsonRPCServer::packDevice(Device *device)
{
    QVariantMap variant;
    variant.insert("id", device->id());
    variant.insert("deviceClassId", device->deviceClassId());
    variant.insert("name", device->name());
    variant.insert("params", device->params());
    return variant;
}

QVariantMap JsonRPCServer::packAction(const Action &action)
{
    QVariantMap variant;
    variant.insert("id", action.id());
    variant.insert("deviceId", action.deviceId());
    variant.insert("name", action.name());
    variant.insert("params", action.params());
    return variant;
}

void JsonRPCServer::sendResponse(int clientId, int commandId, const QVariantMap &params)
{
    QVariantMap rsp;
    rsp.insert("id", commandId);
    rsp.insert("status", "success");
    rsp.insert("params", params);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(rsp);
    m_tcpServer->sendResponse(clientId, jsonDoc.toJson());
}

void JsonRPCServer::sendErrorResponse(int clientId, int commandId, const QString &error)
{
    QVariantMap rsp;
    rsp.insert("id", commandId);
    rsp.insert("status", "error");
    rsp.insert("error", error);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(rsp);
    m_tcpServer->sendResponse(clientId, jsonDoc.toJson());
}


