/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
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
