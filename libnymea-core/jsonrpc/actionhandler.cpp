/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::ActionHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Actions namespace.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Action} namespace of the API.

    \sa Action, JsonHandler, JsonRPCServer
*/

#include "actionhandler.h"

#include "nymeacore.h"
#include "devicemanager.h"
#include "types/action.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l ActionHandler with the given \a parent. */
ActionHandler::ActionHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("ExecuteAction", "Execute a single action.");
    setParams("ExecuteAction", JsonTypes::actionDescription());
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("ExecuteAction", returns);

    params.clear(); returns.clear();
    setDescription("GetActionType", "Get the ActionType for the given ActionTypeId");
    params.insert("actionTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetActionType", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:actionType", JsonTypes::actionTypeDescription());
    setReturns("GetActionType", returns);

    connect(NymeaCore::instance(), &NymeaCore::actionExecuted, this, &ActionHandler::actionExecuted);
}

/*! Returns the name of the \l{ActionHandler}. In this case \b Actions.*/
QString ActionHandler::name() const
{
    return "Actions";
}

JsonReply* ActionHandler::ExecuteAction(const QVariantMap &params)
{
    DeviceId deviceId(params.value("deviceId").toString());
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    ParamList actionParams = JsonTypes::unpackParams(params.value("params").toList());

    Action action(actionTypeId, deviceId);
    action.setParams(actionParams);

    DeviceManager::DeviceError status = NymeaCore::instance()->executeAction(action);
    if (status == DeviceManager::DeviceErrorAsync) {
        JsonReply *reply = createAsyncReply("ExecuteAction");
        ActionId id = action.id();
        connect(reply, &JsonReply::finished, [this, id](){ m_asyncActionExecutions.remove(id); });
        m_asyncActionExecutions.insert(id, reply);
        return reply;
    }

    return createReply(statusToReply(status));
}

JsonReply *ActionHandler::GetActionType(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc) << "asked for action type" << params;
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const ActionType &actionType, deviceClass.actionTypes()) {
            if (actionType.id() == actionTypeId) {
                QVariantMap data = statusToReply(DeviceManager::DeviceErrorNoError);
                data.insert("actionType", JsonTypes::packActionType(actionType, deviceClass.pluginId(), params.value("locale").toLocale()));
                return createReply(data);
            }
        }
    }
    return createReply(statusToReply(DeviceManager::DeviceErrorActionTypeNotFound));
}

void ActionHandler::actionExecuted(const ActionId &id, DeviceManager::DeviceError status)
{
    if (!m_asyncActionExecutions.contains(id)) {
        return; // Not the action we are waiting for.
    }

    JsonReply *reply = m_asyncActionExecutions.take(id);
    reply->setData(statusToReply(status));
    reply->finished();
}

}
