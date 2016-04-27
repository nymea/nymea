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

#ifndef NETATMOOUTDOORMODULE_H
#define NETATMOOUTDOORMODULE_H

#include <QObject>

class NetatmoOutdoorModule : public QObject
{
    Q_OBJECT
public:
    explicit NetatmoOutdoorModule(const QString &name, const QString &macAddress, const QString &connectionId, const QString &baseStation, QObject *parent = 0);

    // Params
    QString name() const;
    QString macAddress() const;
    QString connectionId() const;
    QString baseStation() const;

    // States
    int lastUpdate() const;
    int humidity() const;
    double temperature() const;
    double minTemperature() const;
    double maxTemperature() const;
    int signalStrength() const;
    int battery() const;

    void updateStates(const QVariantMap &data);

private:
    // Params
    QString m_name;
    QString m_macAddress;
    QString m_connectionId;
    QString m_baseStation;

    // States
    int m_lastUpdate;
    int m_humidity;
    double m_temperature;
    double m_minTemperature;
    double m_maxTemperature;
    int m_signalStrength;
    int m_battery;

signals:
    void statesChanged();

};

#endif // NETATMOOUTDOORMODULE_H
