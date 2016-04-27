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

#include "netatmobasestation.h"

#include <QVariantMap>

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
    // check data timestamp
    if (data.contains("last_status_store")) {
        m_lastUpdate = data.value("last_status_store").toInt();
    }

    // update dashboard data
    if (data.contains("dashboard_data")) {
        QVariantMap measurments = data.value("dashboard_data").toMap();
        m_pressure = measurments.value("AbsolutePressure").toDouble();
        m_humidity = measurments.value("Humidity").toInt();
        m_noise = measurments.value("Noise").toInt();
        m_temperature = measurments.value("Temperature").toDouble();
        m_minTemperature = measurments.value("min_temp").toDouble();
        m_maxTemperature = measurments.value("max_temp").toDouble();
        m_co2 = measurments.value("CO2").toInt();
    }
    // update wifi signal strength
    if (data.contains("wifi_status")) {
        int wifiStrength = data.value("wifi_status").toInt();
        if (wifiStrength <= 56) {
            m_wifiStrength = 100;
        } else if (wifiStrength >= 86) {
            m_wifiStrength = 0;
        } else {
            int delta = 30 - (wifiStrength - 56);
            m_wifiStrength = qRound(100.0 * delta / 30.0);
        }
    }
    emit statesChanged();
}
