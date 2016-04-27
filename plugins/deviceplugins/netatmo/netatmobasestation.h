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

#ifndef NETATMOBASESTATION_H
#define NETATMOBASESTATION_H

#include <QObject>
#include <QString>

class NetatmoBaseStation : public QObject
{
    Q_OBJECT
public:
    explicit NetatmoBaseStation(const QString &name, const QString &macAddress, const QString &connectionId, QObject *parent = 0);

    // Params
    QString name() const;
    QString macAddress() const;
    QString connectionId() const;

    // States
    int lastUpdate() const;
    double temperature() const;
    double minTemperature() const;
    double maxTemperature() const;
    double pressure() const;
    int humidity() const;
    int noise() const;
    int co2() const;
    int wifiStrength() const;

    void updateStates(const QVariantMap &data);

private:
    // Params
    QString m_name;
    QString m_macAddress;
    QString m_connectionId;

    // States
    int m_lastUpdate;
    double m_temperature;
    double m_minTemperature;
    double m_maxTemperature;
    double m_pressure;
    int m_humidity;
    int m_noise;
    int m_co2;
    int m_wifiStrength;

signals:
    void statesChanged();

};

#endif // NETATMOBASESTATION_H
