/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "radiatorthermostat.h"

RadiatorThermostat::RadiatorThermostat(QObject *parent) :
    MaxDevice(parent)
{
}

double RadiatorThermostat::comfortTemp() const
{
    return m_comfortTemp;
}

void RadiatorThermostat::setComfortTemp(const double &comfortTemp)
{
    m_comfortTemp = comfortTemp;
}

double RadiatorThermostat::ecoTemp() const
{
    return m_ecoTemp;
}

void RadiatorThermostat::setEcoTemp(const double &ecoTemp)
{
    m_ecoTemp = ecoTemp;
}

double RadiatorThermostat::maxSetPointTemp() const
{
    return m_maxSetPointTemp;
}

void RadiatorThermostat::setMaxSetPointTemp(const double &maxSetPointTemp)
{
    m_maxSetPointTemp = maxSetPointTemp;
}

double RadiatorThermostat::minSetPointTemp() const
{
    return m_minSetPointTemp;
}

void RadiatorThermostat::setMinSetPointTemp(const double &minSetPointTemp)
{
    m_minSetPointTemp = minSetPointTemp;
}

double RadiatorThermostat::windowOpenTemp() const
{
    return m_windowOpenTemp;
}

void RadiatorThermostat::setWindowOpenTemp(const double &windowOpenTemp)
{
    m_windowOpenTemp = windowOpenTemp;
}

double RadiatorThermostat::offsetTemp() const
{
    return m_offsetTemp;
}

void RadiatorThermostat::setOffsetTemp(const double &offsetTemp)
{
    m_offsetTemp = offsetTemp;
}

int RadiatorThermostat::windowOpenDuration() const
{
    return m_windowOpenDuration;
}

void RadiatorThermostat::setWindowOpenDuration(const int &windowOpenDuration)
{
    m_windowOpenDuration = windowOpenDuration;
}

int RadiatorThermostat::boostDuration() const
{
    return m_boostDuration;
}

void RadiatorThermostat::setBoostDuration(const int &boostDuration)
{
    m_boostDuration = boostDuration;
}

int RadiatorThermostat::boostValveValue() const
{
    return m_boostValveValue;
}

void RadiatorThermostat::setBoostValveValue(const int &boostValveValue)
{
    m_boostValveValue = boostValveValue;
}

QString RadiatorThermostat::discalcingWeekDay() const
{
    return m_discalcWeekDay;
}

void RadiatorThermostat::setDiscalcingWeekDay(const QString &discalcingWeekDay)
{
    m_discalcWeekDay = discalcingWeekDay;
}

QTime RadiatorThermostat::discalcingTime() const
{
    return m_discalcTime;
}

void RadiatorThermostat::setDiscalcingTime(const QTime &discalcingTime)
{
    m_discalcTime = discalcingTime;
}

double RadiatorThermostat::valveMaximumSettings() const
{
    return m_valveMaximumSettings;
}

void RadiatorThermostat::setValveMaximumSettings(const double &valveMaximumSettings)
{
    m_valveMaximumSettings = valveMaximumSettings;
}

double RadiatorThermostat::valveOffset() const
{
    return m_valveOffset;
}

void RadiatorThermostat::setValveOffset(const double &valveOffset)
{
    m_valveOffset = valveOffset;
}

bool RadiatorThermostat::informationValid() const
{
    return m_informationValid;
}

void RadiatorThermostat::setInformationValid(const bool &informationValid)
{
    m_informationValid = informationValid;
}

bool RadiatorThermostat::errorOccured() const
{
    return m_errorOccured;
}

void RadiatorThermostat::setErrorOccured(const bool &errorOccured)
{
    m_errorOccured = errorOccured;
}

bool RadiatorThermostat::isAnswereToCommand() const
{
    return m_isAnswerToCommand;
}

void RadiatorThermostat::setIsAnswereToCommand(const bool &isAnswereToCommand)
{
    m_isAnswerToCommand = isAnswereToCommand;
}

bool RadiatorThermostat::initialized() const
{
    return m_initialized;
}

void RadiatorThermostat::setInitialized(const bool &initialized)
{
    m_initialized = initialized;
}

bool RadiatorThermostat::batteryLow() const
{
    return m_batteryLow;
}

void RadiatorThermostat::setBatteryLow(const bool &batteryLow)
{
    m_batteryLow = batteryLow;
}

bool RadiatorThermostat::linkStatusOK() const
{
    return m_linkStatusOK;
}

void RadiatorThermostat::setLinkStatusOK(const bool &linkStatusOK)
{
    m_linkStatusOK = linkStatusOK;
}

bool RadiatorThermostat::panelLocked() const
{
    return m_panelLocked;
}

void RadiatorThermostat::setPanelLocked(const bool &panelLocked)
{
    m_panelLocked = panelLocked;
}

bool RadiatorThermostat::gatewayKnown() const
{
    return m_gatewayKnown;
}

void RadiatorThermostat::setGatewayKnown(const bool &gatewayKnown)
{
    m_gatewayKnown = gatewayKnown;
}

bool RadiatorThermostat::dtsActive() const
{
    return m_dtsActive;
}

void RadiatorThermostat::setDtsActive(const bool &dtsActive)
{
    m_dtsActive = dtsActive;
}

int RadiatorThermostat::deviceMode() const
{
    return m_deviceMode;
}

void RadiatorThermostat::setDeviceMode(const int &deviceMode)
{
    m_deviceMode = deviceMode;

    switch (deviceMode) {
    case Auto:
        m_deviceModeString = "Auto";
        break;
    case Manual:
        m_deviceModeString = "Manuel";
        break;
    case Temporary:
        m_deviceModeString = "Temporary";
        break;
    case Boost:
        m_deviceModeString = "Boost";
        break;
    default:
        m_deviceModeString = "-";
        break;
    }
}

QString RadiatorThermostat::deviceModeString() const
{
    return m_deviceModeString;
}

int RadiatorThermostat::valvePosition() const
{
    return m_valvePosition;
}

void RadiatorThermostat::setValvePosition(const int &valvePosition)
{
    m_valvePosition = valvePosition;
}

double RadiatorThermostat::setpointTemperature() const
{
    return m_setpointTemperature;
}

void RadiatorThermostat::setSetpointTemperatre(const double &setpointTemperature)
{
    m_setpointTemperature = setpointTemperature;
}

