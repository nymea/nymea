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

#ifndef DEVICEPLUGINDATETIME_H
#define DEVICEPLUGINDATETIME_H

#include "plugin/deviceplugin.h"
#include "alarm.h"

#include <QDateTime>
#include <QTimeZone>
#include <QTime>
#include <QTimer>

class DevicePluginDateTime : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugindatetime.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginDateTime();

    DeviceManager::HardwareResources requiredHardware() const override;
    QList<ParamType> configurationDescription() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void postSetupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void networkManagerReplyReady(QNetworkReply *reply) override;

    void startMonitoringAutoDevices() override;

private:
    QTimer *m_timer;
    Device *m_todayDevice;
    QTimeZone m_timeZone;
    QDateTime m_currentDateTime;

    QHash<Device *, Alarm *> m_alarms;

    QDateTime m_dusk;
    QDateTime m_sunrise;
    QDateTime m_noon;
    QDateTime m_dawn;
    QDateTime m_sunset;


    QList<QNetworkReply *> m_locationReplies;
    QList<QNetworkReply *> m_timeReplies;

    void searchGeoLocation();
    void processGeoLocationData(const QByteArray &data);

    void getTimes(const QString &latitude, const QString &longitude);
    void processTimesData(const QByteArray &data);

signals:
    void dusk();
    void sunset();
    void noon();
    void sunrise();
    void dawn();
    void minuteChanged();
    void hourChanged();
    void dayChanged();

private slots:
    void onTimeout();
    void onAlarm();
    void onMinuteChanged();
    void onHourChanged();
    void onDayChanged();

    void updateTimes();

    void onConfigValueChanged(const QString &paramName, const QVariant &value);
    void validateTimeTypes(const QDateTime &dateTime);

};

#endif // DEVICEPLUGINDATETIME_H
