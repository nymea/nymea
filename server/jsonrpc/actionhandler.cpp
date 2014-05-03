/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "actionhandler.h"

#include "guhcore.h"
#include "devicemanager.h"
#include "types/action.h"

#include <QDebug>

ActionHandler::ActionHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("ExecuteAction", "Execute a single action.");
    setParams("ExecuteAction", JsonTypes::actionDescription());
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    setReturns("ExecuteAction", returns);

    connect(GuhCore::instance()->deviceManager(), &DeviceManager::actionExecutionFinished, this, &ActionHandler::actionExecuted);
}

QString ActionHandler::name() const
{
    return "Actions";
}

JsonReply* ActionHandler::ExecuteAction(const QVariantMap &params)
{

    DeviceId deviceId(params.value("deviceId").toString());
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    QList<Param> actionParams = JsonTypes::unpackParams(params.value("params").toList());

    Action action(actionTypeId, deviceId);
    action.setParams(actionParams);

    qDebug() << "actions params in json" << action.params() << params;


    QPair<DeviceManager::DeviceError, QString> status = GuhCore::instance()->deviceManager()->executeAction(action);
    if (status.first == DeviceManager::DeviceErrorAsync) {
        JsonReply *reply = createAsyncReply("ExecuteAction");
        m_asyncActionExecutions.insert(action.id(), reply);
        return reply;
    }

    QVariantMap returns = statusToReply(status.first, status.second);
    return createReply(returns);
}

void ActionHandler::actionExecuted(const ActionId &id, DeviceManager::DeviceError status, const QString &errorMessage)
{
    if (!m_asyncActionExecutions.contains(id)) {
        return; // Not the action we are waiting for.
    }

    JsonReply *reply = m_asyncActionExecutions.take(id);
    reply->setData(statusToReply(status, errorMessage));
    reply->finished();
}

QVariantMap ActionHandler::statusToReply(DeviceManager::DeviceError status, const QString &errorMessage)
{
    QVariantMap returns;

    switch (status) {
    case DeviceManager::DeviceErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        break;
    case DeviceManager::DeviceErrorDeviceNotFound:
        returns.insert("errorMessage", QString("No such device: %1").arg(errorMessage));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorActionTypeNotFound:
        returns.insert("errorMessage", QString("ActionType not found: %1").arg(errorMessage));
        returns.insert("success", false);
        break;
    case DeviceManager::DeviceErrorMissingParameter:
        returns.insert("errorMessage", QString("Missing parameter: %1").arg(errorMessage));
        returns.insert("success", false);
        break;
    default:
        returns.insert("errorMessage", QString("Unknown error %1 %2").arg(status).arg(errorMessage));
        returns.insert("success", false);
    }
    return returns;
}
