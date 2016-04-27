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

#include "alarm.h"
#include "extern-plugininfo.h"

Alarm::Alarm(QObject *parent) :
    QObject(parent),
    m_duskOffset(QDateTime()),
    m_sunriseOffset(QDateTime()),
    m_noonOffset(QDateTime()),
    m_sunsetOffset(QDateTime()),
    m_dawnOffset(QDateTime())
{
}

void Alarm::setName(const QString &name)
{
    m_name = name;
}

QString Alarm::name() const
{
    return m_name;
}

void Alarm::setMonday(const bool &monday)
{
    m_monday = monday;
}

bool Alarm::monday() const
{
    return m_monday;
}

void Alarm::setTuesday(const bool &tuesday)
{
    m_tuesday = tuesday;
}

bool Alarm::tuesday() const
{
    return m_tuesday;
}

void Alarm::setWednesday(const bool &wednesday)
{
    m_wednsday = wednesday;
}

bool Alarm::wednesday() const
{
    return m_wednsday;
}

void Alarm::setThursday(const bool &thursday)
{
    m_thursday = thursday;
}

bool Alarm::thursday() const
{
    return m_thursday;
}

void Alarm::setFriday(const bool &friday)
{
    m_friday = friday;
}

bool Alarm::friday() const
{
    return m_friday;
}

void Alarm::setSaturday(const bool &saturday)
{
    m_saturday = saturday;
}

bool Alarm::saturday() const
{
    return m_saturday;
}

void Alarm::setSunday(const bool &sunday)
{
    m_sunday = sunday;
}

bool Alarm::sunday() const
{
    return m_sunday;
}

void Alarm::setMinutes(const int &minutes)
{
    m_minutes = minutes;
}

int Alarm::minutes() const
{
    return m_minutes;
}

void Alarm::setHours(const int &hours)
{
    m_hours = hours;
}

int Alarm::hours() const
{
    return m_hours;
}

void Alarm::setOffset(const int &offset)
{
    m_offset = offset;
}

int Alarm::offset() const
{
    return m_offset;
}

void Alarm::setDusk(const QDateTime &dusk)
{
    m_duskOffset = calculateOffsetTime(dusk);
}

void Alarm::setSunrise(const QDateTime &sunrise)
{
    m_sunriseOffset = calculateOffsetTime(sunrise);
}

void Alarm::setNoon(const QDateTime &noon)
{
    m_noonOffset = calculateOffsetTime(noon);
}

void Alarm::setSunset(const QDateTime &sunset)
{
    m_sunsetOffset = calculateOffsetTime(sunset);
}

void Alarm::setDawn(const QDateTime &dawn)
{
    m_dawnOffset = calculateOffsetTime(dawn);
}

void Alarm::setTimeType(const Alarm::TimeType &timeType)
{
    m_timeType = timeType;
}

void Alarm::setTimeType(const QString &timeType)
{
    if (timeType == "time") {
        m_timeType = TimeTypeTime;
    } else if (timeType == "dusk") {
        m_timeType = TimeTypeDusk;
    } else if (timeType == "sunrise") {
        m_timeType = TimeTypeSunrise;
    } else if (timeType == "sunnoon") {
        m_timeType = TimeTypeNoon;
    } else if (timeType == "sunset") {
        m_timeType = TimeTypeSunset;
    } else if (timeType == "dawn") {
        m_timeType = TimeTypeDawn;
    }
}

Alarm::TimeType Alarm::timeType() const
{
    return m_timeType;
}

QDateTime Alarm::getAlertTime() const
{
    return QDateTime(QDate::currentDate(), QTime(hours(), minutes())).addSecs(m_offset * 60);
}

QDateTime Alarm::calculateOffsetTime(const QDateTime &dateTime) const
{
    return QDateTime(dateTime).addSecs(m_offset * 60);
}

bool Alarm::checkDayOfWeek(const QDateTime &dateTime)
{    
    switch (dateTime.date().dayOfWeek()) {
    case Qt::Monday:
        return monday();
    case Qt::Tuesday:
        return tuesday();
    case Qt::Wednesday:
        return wednesday();
    case Qt::Thursday:
        return thursday();
    case Qt::Friday:
        return friday();
    case Qt::Saturday:
        return saturday();
    case Qt::Sunday:
        return sunday();
    default:
        return false;
    }
}

bool Alarm::checkHour(const QDateTime &dateTime)
{
    if (getAlertTime().time().hour() == dateTime.time().hour()) {
        return true;
    }
    return false;
}

bool Alarm::checkMinute(const QDateTime &dateTime)
{
    if (getAlertTime().time().minute() == dateTime.time().minute()) {
        return true;
    }
    return false;
}

bool Alarm::checkTimeTypes(const QDateTime &dateTime)
{
    bool checkOk = false;
    switch (m_timeType) {
    case TimeTypeTime:
        qCWarning(dcDateTime) << name() << "wrong time type. This should never happen!";
        checkOk = false;
        break;
    case TimeTypeDusk:
        if (m_duskOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "time:" << dateTime.time().toString() << "matches dusk time" << m_duskOffset.time().toString() << "with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeSunrise:
        if (m_sunriseOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "time:" << dateTime.time().toString() << "matches sunrise time" << m_sunriseOffset.time().toString() << "with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeNoon:
        if (m_noonOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "time:" << dateTime.time().toString() << "matches noon time" << m_noonOffset.time().toString() << "with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeSunset:
        if (m_sunsetOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "time:" << dateTime.time().toString() << "matches sunset time" << m_sunsetOffset.time().toString() << "with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeDawn:
        if (m_dawnOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "time:" << dateTime.time().toString() << "matches dawn time" << m_dawnOffset.time().toString() << "with offset" << m_offset;
            checkOk = true;
        }
        break;
    default:
        checkOk = false;
    }
    return checkOk;
}

void Alarm::validate(const QDateTime &dateTime)
{
    if (m_timeType != TimeTypeTime)
        return;

    if (!checkDayOfWeek(dateTime))
        return;

    // check if should use the given time
    if (!checkHour(dateTime))
        return;

    if (!checkMinute(dateTime))
        return;

    qCDebug(dcDateTime) << name() << "time match" << dateTime.time().toString("hh:mm") << QTime(hours(), minutes()).toString("hh:mm") << "with offset" << m_offset;
    emit alarm();
}

void Alarm::validateTimes(const QDateTime &dateTime)
{
    if (m_timeType == TimeTypeTime)
        return;

    if (!checkTimeTypes(dateTime))
        return;

    emit alarm();
}
