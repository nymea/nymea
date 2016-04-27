/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#include "hueremote.h"
#include "extern-plugininfo.h"

HueRemote::HueRemote(QObject *parent) :
    HueDevice(parent)
{
}

int HueRemote::battery() const
{
    return m_battery;
}

void HueRemote::setBattery(const int &battery)
{
    m_battery = battery;
}

void HueRemote::updateStates(const QVariantMap &statesMap, const QVariantMap &configMap)
{
    setReachable(configMap.value("reachable", false).toBool());
    setBattery(configMap.value("battery", 0).toInt());

    emit stateChanged();

    QString lastUpdate = statesMap.value("lastupdated").toString();
    if (m_lastUpdate != lastUpdate) {
        m_lastUpdate = lastUpdate;

        int buttonCode = statesMap.value("buttonevent").toInt();
        qCDebug(dcPhilipsHue) << "button pressed" << buttonCode;

        emit buttonPressed(buttonCode);
    }
}

