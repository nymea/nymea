/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "netatmobasestation.h"

NetatmoBaseStation::NetatmoBaseStation(const QString &name, const QString &macAddress, const QString &connectionId, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_macAddress(macAddress),
    m_connectionId(connectionId)
{
}

QString NetatmoBaseStation::name() const
{
    return m_name;
}

QString NetatmoBaseStation::macAddress() const
{
    return m_macAddress;
}

QString NetatmoBaseStation::connectionId() const
{
    return m_connectionId;
}

int NetatmoBaseStation::lastUpdate() const
{
    return m_lastUpdate;
}

double NetatmoBaseStation::temperature() const
{
    return m_temperature;
}

double NetatmoBaseStation::minTemperature() const
{
    return m_minTemperature;
}

double NetatmoBaseStation::maxTemperature() const
{
    return m_maxTemperature;
}

double NetatmoBaseStation::pressure() const
{
    return m_pressure;
}

int NetatmoBaseStation::humidity() const
{
    return m_humidity;
}

int NetatmoBaseStation::noise() const
{
    return m_noise;
}

int NetatmoBaseStation::co2() const
{
    return m_co2;
}

int NetatmoBaseStation::wifiStrength() const
{
    return m_wifiStrength;
}

void NetatmoBaseStation::updateStates(const QVariantMap &data)
{
    Q_UNUSED(data)
}
