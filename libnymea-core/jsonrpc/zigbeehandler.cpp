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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zigbeehandler.h"

namespace nymeaserver {

ZigbeeHandler::ZigbeeHandler(QObject *parent) :
    JsonHandler(parent)
{
    registerEnum<ZigbeeNetwork::State>();
    registerEnum<ZigbeeNetworkManager::BackendType>();

    QVariantMap params, returns;
    QString description;

    params.clear(); returns.clear();
    description = "Get the status of the current network.";
    returns.insert("available", enumValueName(Bool));
    returns.insert("enabled", enumValueName(Bool));
    returns.insert("configured", enumValueName(Bool));
    returns.insert("state", enumRef<ZigbeeNetwork::State>());
    registerMethod("GetNetworkStatus", description, params, returns);


    // GetUartInterfaces
    params.clear(); returns.clear();
    description = "Get the available UART interfaces in order to set up the zigbee network on the approriate serial interface.";
    returns.insert("available", enumValueName(Bool));
    returns.insert("enabled", enumValueName(Bool));
    returns.insert("configured", enumValueName(Bool));
    returns.insert("state", enumRef<ZigbeeNetwork::State>());
    registerMethod("GetUartInterfaces", description, params, returns);


    // Setup network, uart, baudrate, backendtype



}

QString ZigbeeHandler::name() const
{
    return "Zigbee";
}

}
