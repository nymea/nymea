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
#include "devices/devicemanager.h"
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

    params.clear(); returns.clear();
    setDescription("ExecuteBrowserItem", "Execute the item identified by itemId on the given device.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("itemId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("ExecuteBrowserItem", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("ExecuteBrowserItem", returns);

    params.clear(); returns.clear();
    setDescription("ExecuteBrowserItemAction", "Execute the action for the browser item identified by actionTypeId and the itemId on the given device.");
    params.insert("deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("itemId", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("actionTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:params", QVariantList() << JsonTypes::paramRef());
    setParams("ExecuteBrowserItemAction", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    setReturns("ExecuteBrowserItemAction", returns);

    connect(NymeaCore::instance(), &NymeaCore::actionExecuted, this, &ActionHandler::actionExecuted);
    connect(NymeaCore::instance(), &NymeaCore::browserItemExecuted, this, &ActionHandler::browserItemExecuted);
    connect(NymeaCore::instance(), &NymeaCore::browserItemActionExecuted, this, &ActionHandler::browserItemActionExecuted);
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

    Device::DeviceError status = NymeaCore::instance()->executeAction(action);
    if (status == Device::DeviceErrorAsync) {
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
                QVariantMap data = statusToReply(Device::DeviceErrorNoError);
                data.insert("actionType", JsonTypes::packActionType(actionType, deviceClass.pluginId(), params.value("locale").toLocale()));
                return createReply(data);
            }
        }
    }
    return createReply(statusToReply(Device::DeviceErrorActionTypeNotFound));
}

void ActionHandler::actionExecuted(const ActionId &id, Device::DeviceError status)
{
    if (!m_asyncActionExecutions.contains(id)) {
        return; // Not the action we are waiting for.
    }

    JsonReply *reply = m_asyncActionExecutions.take(id);
    reply->setData(statusToReply(status));
    reply->finished();
}

JsonReply *ActionHandler::ExecuteBrowserItem(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    BrowserAction action(deviceId, itemId);
    Device::DeviceError status = NymeaCore::instance()->executeBrowserItem(action);
    if (status == Device::DeviceErrorAsync) {
        JsonReply *reply = createAsyncReply("ExecuteBrowserItem");
        ActionId id = action.id();
        connect(reply, &JsonReply::finished, [this, id](){ m_asyncActionExecutions.remove(id); });
        m_asyncActionExecutions.insert(id, reply);
        return reply;
    }
    return createReply(statusToReply(status));
}

JsonReply *ActionHandler::ExecuteBrowserItemAction(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    ActionTypeId actionTypeId = ActionTypeId(params.value("actionTypeId").toString());
    ParamList paramList = JsonTypes::unpackParams(params.value("params").toList());
    BrowserItemAction browserItemAction(deviceId, itemId, actionTypeId, paramList);
    Device::DeviceError status = NymeaCore::instance()->executeBrowserItemAction(browserItemAction);
    if (status == Device::DeviceErrorAsync) {
        JsonReply *reply = createAsyncReply("ExecuteBrowserItemAction");
        ActionId id = browserItemAction.id();
        connect(reply, &JsonReply::finished, [this, id](){ m_asyncActionExecutions.remove(id); });
        m_asyncActionExecutions.insert(id, reply);
        return reply;
    }
    return createReply(statusToReply(status));

}

void ActionHandler::browserItemExecuted(const ActionId &id, Device::DeviceError status)
{
    if (!m_asyncActionExecutions.contains(id)) {
        return; // Not the action we are waiting for.
    }

    JsonReply *reply = m_asyncActionExecutions.take(id);
    reply->setData(statusToReply(status));
    reply->finished();
}

void ActionHandler::browserItemActionExecuted(const ActionId &id, Device::DeviceError status)
{
    if (!m_asyncActionExecutions.contains(id)) {
        return; // Not the action we are waiting for.
    }

    JsonReply *reply = m_asyncActionExecutions.take(id);
    reply->setData(statusToReply(status));
    reply->finished();
}


}
