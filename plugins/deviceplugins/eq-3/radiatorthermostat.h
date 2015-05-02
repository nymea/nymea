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

#ifndef RADIATORTHERMOSTAT_H
#define RADIATORTHERMOSTAT_H

#include <QObject>
#include <QTime>

#include "maxdevice.h"

class RadiatorThermostat : public MaxDevice
{
    Q_OBJECT
public:
    explicit RadiatorThermostat(QObject *parent = 0);

    double comfortTemp() const;
    void setComfortTemp(const double &comfortTemp);

    double ecoTemp() const;
    void setEcoTemp(const double &ecoTemp);

    double maxSetPointTemp() const;
    void setMaxSetPointTemp(const double &maxSetPointTemp);

    double minSetPointTemp() const;
    void setMinSetPointTemp(const double &minSetPointTemp);

    double windowOpenTemp() const;
    void setWindowOpenTemp(const double &windowOpenTemp);

    double offsetTemp() const;
    void setOffsetTemp(const double &offsetTemp);

    int windowOpenDuration() const;
    void setWindowOpenDuration(const int &windowOpenDuration);

    int boostDuration() const;
    void setBoostDuration(const int &boostDuration);

    int boostValveValue() const;
    void setBoostValveValue(const int &boostValveValue);

    QString discalcingWeekDay() const;
    void setDiscalcingWeekDay(const QString &discalcingWeekDay);

    QTime discalcingTime() const;
    void setDiscalcingTime(const QTime &discalcingTime);

    double valveMaximumSettings() const;
    void setValveMaximumSettings(const double &valveMaximumSettings);

    double valveOffset() const;
    void setValveOffset(const double &valveOffset);

    bool informationValid() const;
    void setInformationValid(const bool &informationValid);

    bool errorOccured() const;
    void setErrorOccured(const bool &errorOccured);

    bool isAnswereToCommand() const;
    void setIsAnswereToCommand(const bool &isAnswereToCommand);

    bool initialized() const;
    void setInitialized(const bool &initialized);

    bool batteryLow() const;
    void setBatteryLow(const bool &batteryLow);

    bool linkStatusOK() const;
    void setLinkStatusOK(const bool &linkStatusOK);

    bool panelLocked() const;
    void setPanelLocked(const bool &panelLocked);

    bool gatewayKnown() const;
    void setGatewayKnown(const bool &gatewayKnown);

    bool dtsActive() const;
    void setDtsActive(const bool &dtsActive);

    int deviceMode() const;
    void setDeviceMode(const int &deviceMode);

    QString deviceModeString() const;

    int valvePosition() const;
    void setValvePosition(const int &valvePosition);

    double setpointTemperature() const;
    void setSetpointTemperatre(const double &setpointTemperature);

private:
    double m_comfortTemp;
    double m_ecoTemp;
    double m_maxSetPointTemp;
    double m_minSetPointTemp;
    double m_windowOpenTemp;
    double m_offsetTemp;
    int m_windowOpenDuration;
    int m_boostDuration;
    int m_boostValveValue;
    QString m_discalcWeekDay;
    QTime m_discalcTime;
    double m_valveMaximumSettings;
    double m_valveOffset;
    bool m_informationValid;
    bool m_errorOccured;
    bool m_isAnswerToCommand;
    bool m_initialized;
    bool m_batteryLow;
    bool m_linkStatusOK;
    bool m_panelLocked;
    bool m_gatewayKnown;
    bool m_dtsActive;
    int m_deviceMode;
    QString m_deviceModeString;
    int m_valvePosition;
    double m_setpointTemperature;

signals:

public slots:

};

#endif // RADIATORTHERMOSTAT_H
