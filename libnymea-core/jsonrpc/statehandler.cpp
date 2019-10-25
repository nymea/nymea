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
    \class nymeaserver::StateHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt States namespace of the JSON-RPC API.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {States} namespace of the API.

    \sa State, JsonHandler, JsonRPCServer
*/

#include "statehandler.h"
#include "devicehandler.h"
#include "nymeacore.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l{StateHandler} with the given \a parent. */
StateHandler::StateHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap state;
    state.insert("stateTypeId", enumValueName(Uuid));
    state.insert("deviceId", enumValueName(Uuid));
    state.insert("value", enumValueName(Variant));
    registerObject("State", state);

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the StateType for the given stateTypeId.";
    params.insert("stateTypeId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:stateType", objectRef("StateType"));
    registerMethod("GetStateType", description, params, returns, true);
}

/*! Returns the name of the \l{StateHandler}. In this case \b States.*/
QString StateHandler::name() const
{
    return "States";
}

JsonReply* StateHandler::GetStateType(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc) << "asked for state type" << params;
    StateTypeId stateTypeId(params.value("stateTypeId").toString());
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const StateType &stateType, deviceClass.stateTypes()) {
            if (stateType.id() == stateTypeId) {
                QVariantMap data;
                data.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorNoError));
                // FIXME TODO!!!
//                data.insert("stateType", DeviceHandler::packStateType(stateType, deviceClass.pluginId(), params.value("locale").toLocale()));
                return createReply(data);
            }
        }
    }
    QVariantMap data;
    data.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorStateTypeNotFound));
    return createReply(data);
}

}
