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

/*!
    \page datetime.html
    \title Time

    \ingroup plugins
    \ingroup services

    The time plugin allows you create rules based on the time, day, month, year, weekday or on weekend.

    For the correct setup you can configure the time zone in the plugin configuration. The language
    of the "month name" and "weekday name" depends on the locale settings of the system. To have the correct
    time you need \l{https://en.wikipedia.org/wiki/Network_Time_Protocol}{ntp}.

    The weekday integer value stands for:
    \table
    \header
        \li Weekday
        \li int
    \row
        \li Monday
        \li 1
    \row
        \li Tuesday
        \li 2
    \row
        \li Wednesday
        \li 3
    \row
        \li Thursday
        \li 4
    \row
        \li Friday
        \li 5
    \row
        \li Saturday
        \li 6
    \row
        \li Sunday
        \li 7
    \endtable

    The "weekend" \l{State} will be true, if the current weekday is Saturday or Sunday, otherwise it will be false.

    \chapter Today
    The today plugin gives you information about the current day and some special times of the day like
    dawn, sunrise, noon, sunset and dawn. In order to get the correct times of the current day for your location, the plugin needs to know where
    you are. The plugin will autodetect your location according to you wan ip (\l{http://ip-api.com/json}{http://ip-api.com/json})
    and will download the sunset / sunrise times from the online database \l{http://sunrise-sunset.org/}{http://sunrise-sunset.org/}.
    If the configured timezone does not match with the autodetected timezone from the ip the specialdates will be set to 0 (01.01.1970 - 00:00.00).

    \image day-times.png

    Special times of a day (\l{https://en.wikipedia.org/wiki/Twilight#/media/File:Twilight_description_full_day.svg}{original source})

    \chapter Alarm
    The alarm plugin allowes you to define an alarm which depends on a certain time, weekday or special day time like sunrise or
    sunset. An offset can also be definend.

    \chapter Countdown
    The countdown plugin allowes you to define a countown which triggers an \l{Event} on timeout.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/datetime/deviceplugindatetime.json
*/

#include "deviceplugindatetime.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QJsonDocument>
#include <QUrlQuery>

DevicePluginDateTime::DevicePluginDateTime() :
    m_timer(0),
    m_todayDevice(0),
    m_timeZone(QTimeZone(QTimeZone::systemTimeZoneId())),
    m_dusk(QDateTime()),
    m_sunrise(QDateTime()),
    m_noon(QDateTime()),
    m_sunset(QDateTime()),
    m_dawn(QDateTime())
{
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);

    m_currentDateTime = QDateTime(QDate::currentDate(), QTime::currentTime(), m_timeZone);

    connect(m_timer, &QTimer::timeout, this, &DevicePluginDateTime::onSecondChanged);
}

DeviceManager::HardwareResources DevicePluginDateTime::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceSetupStatus DevicePluginDateTime::setupDevice(Device *device)
{
    // check timezone
    if(!m_timeZone.isValid()){
        qCWarning(dcDateTime) << "Invalid time zone.";
        return DeviceManager::DeviceSetupStatusFailure;
    }

    // date
    if (device->deviceClassId() == todayDeviceClassId) {
        if (m_todayDevice != 0) {
            qCWarning(dcDateTime) << "There is already a date device or not deleted correctly! this should never happen!!";
            return DeviceManager::DeviceSetupStatusFailure;
        }
        m_todayDevice = device;
        qCDebug(dcDateTime) << "Create today device: current time" << m_currentDateTime.currentDateTime().toString();
    }

    // alarm
    if (device->deviceClassId() == alarmDeviceClassId) {
        Alarm *alarm = new Alarm(this);
        alarm->setName(device->name());
        alarm->setMonday(device->paramValue("monday").toBool());
        alarm->setTuesday(device->paramValue("tuesday").toBool());
        alarm->setWednesday(device->paramValue("wednesday").toBool());
        alarm->setThursday(device->paramValue("thursday").toBool());
        alarm->setFriday(device->paramValue("friday").toBool());
        alarm->setSaturday(device->paramValue("saturday").toBool());
        alarm->setSunday(device->paramValue("sunday").toBool());
        alarm->setMinutes(device->paramValue("minutes").toInt());
        alarm->setHours(device->paramValue("hours").toInt());
        alarm->setTimeType(device->paramValue("time type").toString());
        alarm->setOffset(device->paramValue("offset").toInt());
        alarm->setDusk(m_dusk);
        alarm->setSunrise(m_sunrise);
        alarm->setNoon(m_noon);
        alarm->setDawn(m_dawn);
        alarm->setSunset(m_sunset);

        connect(alarm, &Alarm::alarm, this, &DevicePluginDateTime::onAlarm);

        m_alarms.insert(device, alarm);
    }

    if (device->deviceClassId() == countdownDeviceClassId) {
        Countdown *countdown = new Countdown(device->name(),
                                             QTime(device->paramValue("hours").toInt(),
                                                   device->paramValue("minutes").toInt(),
                                                   device->paramValue("seconds").toInt()),
                                             device->paramValue("repeating").toBool());

        connect(countdown, &Countdown::countdownTimeout, this, &DevicePluginDateTime::onCountdownTimeout);
        connect(countdown, &Countdown::runningStateChanged, this, &DevicePluginDateTime::onCountdownRunningChanged);

        qCDebug(dcDateTime) << "Setup countdown" << countdown->name() << countdown->time().toString();
        m_countdowns.insert(device, countdown);
    }

    m_timer->start();

    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginDateTime::postSetupDevice(Device *device)
{
    if (device->deviceClassId() == todayDeviceClassId) {
        QDateTime zoneTime = QDateTime::currentDateTime().toTimeZone(m_timeZone);
        updateTimes();
        onMinuteChanged(zoneTime);
        onHourChanged(zoneTime);
        onDayChanged(zoneTime);
    }
}

void DevicePluginDateTime::deviceRemoved(Device *device)
{
    // check if we still need the timer
    if (myDevices().count() == 0) {
        m_timer->stop();
    }

    // date
    if (device->deviceClassId() == todayDeviceClassId) {
        m_todayDevice = 0;
    }

    // alarm
    if (device->deviceClassId() == alarmDeviceClassId) {
        Alarm *alarm = m_alarms.take(device);
        alarm->deleteLater();
    }

    // countdown
    if (device->deviceClassId() == countdownDeviceClassId) {
        Countdown *countdown = m_countdowns.take(device);
        countdown->deleteLater();
    }

    startMonitoringAutoDevices();
}

DeviceManager::DeviceError DevicePluginDateTime::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == countdownDeviceClassId) {
        Countdown *countdown = m_countdowns.value(device);
        if (action.actionTypeId() == startActionTypeId) {
            countdown->start();
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == restartActionTypeId) {
            countdown->restart();
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == stopActionTypeId) {
            countdown->stop();
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginDateTime::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_locationReplies.contains(reply)) {
        m_locationReplies.removeAll(reply);
        if (status != 200) {
            qCWarning(dcDateTime) << "Http error status for location request:" << status << reply->error();
        } else {
            processGeoLocationData(reply->readAll());
        }
    } else if (m_timeReplies.contains(reply)) {
        m_timeReplies.removeAll(reply);
        if (status != 200) {
            qCWarning(dcDateTime) << "Http error status for time request:" << status << reply->error();
        } else {
            processTimesData(reply->readAll());
        }
    }
    reply->deleteLater();
}

void DevicePluginDateTime::startMonitoringAutoDevices()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == todayDeviceClassId) {
            return; // We already have the date device... do nothing.
        }
    }

    DeviceDescriptor dateDescriptor(todayDeviceClassId, "Date", "Time");
    emit autoDevicesAppeared(todayDeviceClassId, QList<DeviceDescriptor>() << dateDescriptor);
}

void DevicePluginDateTime::searchGeoLocation()
{
    if (m_todayDevice == 0)
        return;

    QNetworkRequest request;
    request.setUrl(QUrl("http://ip-api.com/json"));

    qCDebug(dcDateTime) << "Requesting geo location.";

    QNetworkReply *reply = networkManagerGet(request);
    m_locationReplies.append(reply);
}

void DevicePluginDateTime::processGeoLocationData(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcDateTime) << "failed to parse location JSON data:" << error.errorString() << ":" << data ;
        return;
    }

    //qCDebug(dcDateTime) << "geo location data received:" << jsonDoc.toJson();
    QVariantMap response = jsonDoc.toVariant().toMap();
    if (response.value("status") != "success") {
        qCWarning(dcDateTime) << "failed to request geo location:" << response.value("status");
    }

    // check timezone
    QString timeZone = response.value("timezone").toString();

    m_todayDevice->setStateValue(timeZoneStateTypeId, timeZone);
    m_todayDevice->setStateValue(cityStateTypeId, response.value("city").toString());
    m_todayDevice->setStateValue(countryStateTypeId, response.value("country").toString());

    qCDebug(dcDateTime) << "---------------------------------------------";
    qCDebug(dcDateTime) << "autodetected location for" << response.value("query").toString();
    qCDebug(dcDateTime) << " city     :" << response.value("city").toString();
    qCDebug(dcDateTime) << " country  :" << response.value("country").toString();
    qCDebug(dcDateTime) << " code     :" << response.value("countryCode").toString();
    qCDebug(dcDateTime) << " zip code :" << response.value("zip").toString();
    qCDebug(dcDateTime) << " lon      :" << response.value("lon").toByteArray();
    qCDebug(dcDateTime) << " lat      :" << response.value("lat").toByteArray();
    qCDebug(dcDateTime) << "---------------------------------------------";

    getTimes(response.value("lat").toString(), response.value("lon").toString());
}

void DevicePluginDateTime::getTimes(const QString &latitude, const QString &longitude)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("lat", latitude);
    urlQuery.addQueryItem("lng", longitude);
    urlQuery.addQueryItem("date", "today");

    QUrl url = QUrl("http://api.sunrise-sunset.org/json");
    url.setQuery(urlQuery.toString());

    QNetworkRequest request;
    request.setUrl(url);

    QNetworkReply *reply = networkManagerGet(request);
    m_timeReplies.append(reply);
}

void DevicePluginDateTime::processTimesData(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcDateTime) << "failed to parse time JSON data:" << error.errorString() << ":" << data ;
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();
    if (response.value("status") != "OK") {
        qCWarning(dcDateTime) << "failed to request time data:" << response.value("status");
        return;
    }

    // given time is always in UTC
    QVariantMap result = response.value("results").toMap();
    QString dawnString = result.value("civil_twilight_begin").toString();
    QString sunriseString = result.value("sunrise").toString();
    QString noonString = result.value("solar_noon").toString();
    QString sunsetString = result.value("sunset").toString();
    QString duskString = result.value("civil_twilight_end").toString();

    m_dawn = QDateTime(QDate::currentDate(), QTime::fromString(dawnString, "h:m:s AP"), Qt::UTC).toTimeZone(m_timeZone);
    m_sunrise = QDateTime(QDate::currentDate(), QTime::fromString(sunriseString, "h:m:s AP"), Qt::UTC).toTimeZone(m_timeZone);
    m_noon = QDateTime(QDate::currentDate(), QTime::fromString(noonString, "h:m:s AP"), Qt::UTC).toTimeZone(m_timeZone);
    m_sunset = QDateTime(QDate::currentDate(), QTime::fromString(sunsetString, "h:m:s AP"), Qt::UTC).toTimeZone(m_timeZone);
    m_dusk = QDateTime(QDate::currentDate(), QTime::fromString(duskString, "h:m:s AP"), Qt::UTC).toTimeZone(m_timeZone);

    // calculate the times in each alarm
    //    qCDebug(dcDateTime) << " dawn     :" << m_dawn.toString();
    //    qCDebug(dcDateTime) << " sunrise  :" << m_sunrise.toString();
    //    qCDebug(dcDateTime) << " noon     :" << m_noon.toString();
    //    qCDebug(dcDateTime) << " sunset   :" << m_sunset.toString();
    //    qCDebug(dcDateTime) << " dusk     :" << m_dusk.toString();
    //    qCDebug(dcDateTime) << "---------------------------------------------";

    updateTimes();
}

void DevicePluginDateTime::onAlarm()
{
    Alarm *alarm = static_cast<Alarm *>(sender());
    Device *device = m_alarms.key(alarm);

    emit emitEvent(Event(alarmEventTypeId, device->id()));
}

void DevicePluginDateTime::onCountdownTimeout()
{
    Countdown *countdown = static_cast<Countdown *>(sender());
    Device *device = m_countdowns.key(countdown);

    emit emitEvent(Event(timeoutEventTypeId, device->id()));
}

void DevicePluginDateTime::onCountdownRunningChanged(const bool &running)
{
    Countdown *countdown = static_cast<Countdown *>(sender());
    Device *device = m_countdowns.key(countdown);

    device->setStateValue(runningStateTypeId, running);
}

void DevicePluginDateTime::onSecondChanged()
{
    QDateTime currentTime = QDateTime::currentDateTime().toTimeZone(m_timeZone);
    // make shore that ms are 0
    QDateTime zoneTime = QDateTime(QDate(currentTime.date()), QTime(currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second(), 0));

    validateTimeTypes(zoneTime);

    if (zoneTime.date() != m_currentDateTime.date())
        onDayChanged(zoneTime);

    if (zoneTime.time().hour() != m_currentDateTime.time().hour())
        onHourChanged(zoneTime);

    if (zoneTime.time().minute() != m_currentDateTime.time().minute())
        onMinuteChanged(zoneTime);

    // just store for compairing
    m_currentDateTime = zoneTime;
}

void DevicePluginDateTime::onMinuteChanged(const QDateTime &dateTime)
{
    //qCDebug(dcDateTime) << "minute changed" << dateTime.toString();

    // validate alerts
    foreach (Alarm *alarm, m_alarms.values()) {
        alarm->validate(dateTime);
    }
}

void DevicePluginDateTime::onHourChanged(const QDateTime &dateTime)
{
    Q_UNUSED(dateTime)
    //qCDebug(dcDateTime) << "hour changed" <<  dateTime.toString();
    // check every hour in case we are offline in the wrong moment
    searchGeoLocation();
}

void DevicePluginDateTime::onDayChanged(const QDateTime &dateTime)
{
    qCDebug(dcDateTime) << "day changed" << dateTime.toString();

    if (m_todayDevice == 0)
        return;

    m_todayDevice->setStateValue(dayStateTypeId, dateTime.date().day());
    m_todayDevice->setStateValue(monthStateTypeId, dateTime.date().month());
    m_todayDevice->setStateValue(yearStateTypeId, dateTime.date().year());
    m_todayDevice->setStateValue(weekdayStateTypeId, dateTime.date().dayOfWeek());
    m_todayDevice->setStateValue(weekdayNameStateTypeId, dateTime.date().longDayName(dateTime.date().dayOfWeek()));
    m_todayDevice->setStateValue(monthNameStateTypeId, dateTime.date().longMonthName(dateTime.date().month()));
    if(dateTime.date().dayOfWeek() == 6 || dateTime.date().dayOfWeek() == 7){
        m_todayDevice->setStateValue(weekendStateTypeId, true);
    }else{
        m_todayDevice->setStateValue(weekendStateTypeId, false);
    }
}

void DevicePluginDateTime::updateTimes()
{
    // alarms
    foreach (Alarm *alarm, m_alarms.values()) {
        alarm->setDusk(m_dusk);
        alarm->setSunrise(m_sunrise);
        alarm->setNoon(m_noon);
        alarm->setDawn(m_dawn);
        alarm->setSunset(m_sunset);
    }

    // date
    if (m_todayDevice == 0)
        return;

    if (m_dusk.isValid()) {
        m_todayDevice->setStateValue(duskStateTypeId, m_dusk.toTime_t());
    } else {
        m_todayDevice->setStateValue(duskStateTypeId, 0);
    }
    if (m_dusk.isValid()) {
        m_todayDevice->setStateValue(sunriseStateTypeId, m_sunrise.toTime_t());
    } else {
        m_todayDevice->setStateValue(sunriseStateTypeId, 0);
    }
    if (m_dusk.isValid()) {
        m_todayDevice->setStateValue(noonStateTypeId, m_noon.toTime_t());
    } else {
        m_todayDevice->setStateValue(noonStateTypeId, 0);
    }
    if (m_dusk.isValid()) {
        m_todayDevice->setStateValue(sunsetStateTypeId, m_sunset.toTime_t());
    } else {
        m_todayDevice->setStateValue(sunsetStateTypeId, 0);
    }
    if (m_dusk.isValid()) {
        m_todayDevice->setStateValue(dawnStateTypeId, m_dawn.toTime_t());
    } else {
        m_todayDevice->setStateValue(dawnStateTypeId, 0);
    }
}


void DevicePluginDateTime::validateTimeTypes(const QDateTime &dateTime)
{
    if (m_todayDevice == 0)
        return;

    if (dateTime == m_dusk) {
        emit emitEvent(Event(duskEventTypeId, m_todayDevice->id()));
    } else if (dateTime == m_sunrise) {
        emit emitEvent(Event(sunriseEventTypeId, m_todayDevice->id()));
    } else if (dateTime == m_noon) {
        emit emitEvent(Event(noonEventTypeId, m_todayDevice->id()));
    } else if (dateTime == m_dawn) {
        emit emitEvent(Event(dawnEventTypeId, m_todayDevice->id()));
    } else if (dateTime == m_sunset) {
        emit emitEvent(Event(sunsetEventTypeId, m_todayDevice->id()));
    }

    foreach (Alarm *alarm, m_alarms.values()) {
        alarm->validateTimes(dateTime);
    }
}
