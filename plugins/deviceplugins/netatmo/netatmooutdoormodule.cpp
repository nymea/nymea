/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "netatmooutdoormodule.h"

#include <QVariantMap>

NetatmoOutdoorModule::NetatmoOutdoorModule(const QString &name, const QString &macAddress, const QString &connectionId, const QString &baseStation, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_macAddress(macAddress),
    m_connectionId(connectionId),
    m_baseStation(baseStation)
{
}

QString NetatmoOutdoorModule::name() const
{
    return m_name;
}

QString NetatmoOutdoorModule::macAddress() const
{
    return m_macAddress;
}

QString NetatmoOutdoorModule::connectionId() const
{
    return m_connectionId;
}

QString NetatmoOutdoorModule::baseStation() const
{
    return m_baseStation;
}

int NetatmoOutdoorModule::lastUpdate() const
{
    return m_lastUpdate;
}

int NetatmoOutdoorModule::humidity() const
{
    return m_humidity;
}

double NetatmoOutdoorModule::temperature() const
{
    return m_temperature;
}

double NetatmoOutdoorModule::minTemperature() const
{
    return m_minTemperature;
}

double NetatmoOutdoorModule::maxTemperature() const
{
    return m_maxTemperature;
}

int NetatmoOutdoorModule::signalStrength() const
{
    return m_signalStrength;
}

int NetatmoOutdoorModule::battery() const
{
    return m_battery;
}

void NetatmoOutdoorModule::updateStates(const QVariantMap &data)
{
    // check data timestamp
    if (data.contains("last_message")) {
        m_lastUpdate = data.value("last_message").toInt();
    }

    // update dashboard data
    if (data.contains("dashboard_data")) {
        QVariantMap measurments = data.value("dashboard_data").toMap();
        m_humidity = measurments.value("Humidity").toInt();
        m_temperature = measurments.value("Temperature").toDouble();
        m_minTemperature = measurments.value("min_temp").toDouble();
        m_maxTemperature = measurments.value("max_temp").toDouble();
    }
    // update battery strength
    if (data.contains("battery_vp")) {
        int battery = data.value("battery_vp").toInt();
        if (battery >= 6000) {
            m_battery = 100;
        } else if (battery <= 3600) {
            m_battery = 0;
        } else {
            int delta = battery - 3600;
            m_battery = qRound(100.0 * delta / 2400);
        }
    }

    // update signal strength
    if (data.contains("rf_status")) {
        int signalStrength = data.value("rf_status").toInt();
        if (signalStrength <= 60) {
            m_signalStrength = 100;
        } else if (signalStrength >= 90) {
            m_signalStrength = 0;
        } else {
            int delta = 30 - (signalStrength - 60);
            m_signalStrength = qRound(100.0 * delta / 30.0);
        }
    }
    emit statesChanged();

}
