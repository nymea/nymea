/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
#include "devices/deviceactioninfo.h"
#include "devices/browseractioninfo.h"
#include "devices/browseritemactioninfo.h"
#include "types/action.h"
#include "types/actiontype.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l ActionHandler with the given \a parent. */
ActionHandler::ActionHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Enums
    registerEnum<Types::InputType>();
    registerEnum<Types::Unit>();

    // Objects
    registerObject<ParamType, ParamTypes>();
    registerObject<Param, ParamList>();
    registerObject<ActionType>();
    registerObject<Action>();

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Execute a single action.";
    params.insert("actionTypeId", enumValueName(Uuid));
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("o:params", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:displayMessage", enumValueName(String));
    registerMethod("ExecuteAction", description, params, returns, "Please use Devices.ExecuteAction instead.");

    params.clear(); returns.clear();
    description = "Get the ActionType for the given ActionTypeId.";
    params.insert("actionTypeId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:actionType", objectRef<ActionType>());
    registerMethod("GetActionType", description, params, returns, "Please use the Devices namespace instead.");

    params.clear(); returns.clear();
    description = "Execute the item identified by itemId on the given device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("itemId", enumValueName(String));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    registerMethod("ExecuteBrowserItem", description, params, returns, "Please use Devices.ExecuteBrowserItem instead.");

    params.clear(); returns.clear();
    description = "Execute the action for the browser item identified by actionTypeId and the itemId on the given device.";
    params.insert("deviceId", enumValueName(Uuid));
    params.insert("itemId", enumValueName(String));
    params.insert("actionTypeId", enumValueName(Uuid));
    params.insert("o:params", objectRef<ParamList>());
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    registerMethod("ExecuteBrowserItemAction", description, params, returns, "Please use Devices.ExecuteBrowserItem instead.");

}

/*! Returns the name of the \l{ActionHandler}. In this case \b Actions.*/
QString ActionHandler::name() const
{
    return "Actions";
}

JsonReply* ActionHandler::ExecuteAction(const QVariantMap &params, const JsonContext &context)
{
    DeviceId deviceId(params.value("deviceId").toString());
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    ParamList actionParams = unpack<ParamList>(params.value("params"));
    QLocale locale = context.locale();

    Action action(actionTypeId, deviceId);
    action.setParams(actionParams);

    JsonReply *jsonReply = createAsyncReply("ExecuteAction");

    DeviceActionInfo *info = NymeaCore::instance()->executeAction(action);
    connect(info, &DeviceActionInfo::finished, jsonReply, [info, jsonReply, locale](){
        QVariantMap data;
        data.insert("deviceError", enumValueName(info->status()));
        if (!info->displayMessage().isEmpty()) {
            data.insert("displayMessage", info->translatedDisplayMessage(locale));
        }
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *ActionHandler::GetActionType(const QVariantMap &params, const JsonContext &context) const
{
    QLocale locale = context.locale();
    qCDebug(dcJsonRpc) << "asked for action type" << params;
    ActionTypeId actionTypeId(params.value("actionTypeId").toString());
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const ActionType &actionType, deviceClass.actionTypes()) {
            if (actionType.id() == actionTypeId) {
                ActionType translatedActionType = actionType;
                translatedActionType.setDisplayName(NymeaCore::instance()->deviceManager()->translate(deviceClass.pluginId(), actionType.displayName(), locale));

                QVariantMap data;
                data.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorNoError));
                data.insert("actionType", pack(translatedActionType));
                return createReply(data);
            }
        }
    }
    QVariantMap data;
    data.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorActionTypeNotFound));
    return createReply(data);
}

JsonReply *ActionHandler::ExecuteBrowserItem(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    BrowserAction action(deviceId, itemId);

    JsonReply *jsonReply = createAsyncReply("ExecuteBrowserItem");

    BrowserActionInfo *info = NymeaCore::instance()->executeBrowserItem(action);
    connect(info, &BrowserActionInfo::finished, jsonReply, [info, jsonReply](){
        QVariantMap data;
        data.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

JsonReply *ActionHandler::ExecuteBrowserItemAction(const QVariantMap &params)
{
    DeviceId deviceId = DeviceId(params.value("deviceId").toString());
    QString itemId = params.value("itemId").toString();
    ActionTypeId actionTypeId = ActionTypeId(params.value("actionTypeId").toString());
    ParamList paramList = unpack<ParamList>(params.value("params"));
    BrowserItemAction browserItemAction(deviceId, itemId, actionTypeId, paramList);

    JsonReply *jsonReply = createAsyncReply("ExecuteBrowserItemAction");

    BrowserItemActionInfo *info = NymeaCore::instance()->executeBrowserItemAction(browserItemAction);
    connect(info, &BrowserItemActionInfo::finished, jsonReply, [info, jsonReply](){
        QVariantMap data;
        data.insert("deviceError", enumValueName<Device::DeviceError>(info->status()));
        jsonReply->setData(data);
        jsonReply->finished();
    });

    return jsonReply;
}

}
