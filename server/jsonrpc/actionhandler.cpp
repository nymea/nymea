#include "actionhandler.h"

#include "devicemanager.h"
#include "guhcore.h"
#include "action.h"

#include <QDebug>

ActionHandler::ActionHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("ExecuteAction", "Execute a single action.");
    params.insert("actionTypeId", "uuid");
    params.insert("deviceId", "uuid");
    params.insert("params", JsonTypes::paramTypeRef());
    setParams("ExecuteAction", JsonTypes::actionDescription());
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    setReturns("ExecuteAction", returns);
}

QString ActionHandler::name() const
{
    return "Actions";
}

QVariantMap ActionHandler::ExecuteAction(const QVariantMap &params)
{

    QUuid deviceId = params.value("deviceId").toUuid();
    QUuid actionTypeId = params.value("actionTypeId").toUuid();
    QVariantMap actionParams = params.value("params").toMap();

    Action action(deviceId, actionTypeId);
    action.setParams(actionParams);

    qDebug() << "actions params in json" << action.params() << params;


    QVariantMap returns;
    DeviceManager::DeviceError error = GuhCore::instance()->deviceManager()->executeAction(action);

    switch (error) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        break;
    case DeviceManager::DeviceErrorDeviceNotFound:
        returns.insert("errorMessage", "No such device");
        returns.insert("success", false);
        break;
    default:
        returns.insert("errorMessage", "Unknown error.");
        returns.insert("success", false);
    }

    return returns;
}
