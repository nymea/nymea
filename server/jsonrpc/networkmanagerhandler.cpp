/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "networkmanagerhandler.h"
#include "loggingcategories.h"
#include "guhcore.h"
#include "networkmanager/networkmanager.h"


namespace guhserver {

NetworkManagerHandler::NetworkManagerHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetWirelessAccessPoints", "Get the current list of wireless network access points.");
    setParams("GetWirelessAccessPoints", params);
    returns.insert("wirelessAccessPoints", QVariantList() << JsonTypes::wirelessAccessPointRef());
    setReturns("GetWirelessAccessPoints", returns);
}

QString NetworkManagerHandler::name() const
{
    return "NetworkManager";
}

JsonReply *NetworkManagerHandler::GetWirelessAccessPoints(const QVariantMap &params)
{
    Q_UNUSED(params);

    QVariantList wirelessAccessPoints;
    foreach (WirelessAccessPoint *wirelessAccessPoint, GuhCore::instance()->networkManager()->wirelessNetworkManager()->accessPoints())
        wirelessAccessPoints.append(JsonTypes::packWirelessAccessPoint(wirelessAccessPoint));

    QVariantMap returns;
    returns.insert("wirelessAccessPoints", wirelessAccessPoints);
    return createReply(returns);
}

}
