#include "alarm.h"
#include "loggingcategories.h"

Alarm::Alarm(QObject *parent) :
    QObject(parent)
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

void Alarm::setDawn(const QDateTime &dawn)
{
    m_dawnOffset = calculateOffsetTime(dawn);
}

void Alarm::setSunset(const QDateTime &sunset)
{
    m_sunsetOffset = calculateOffsetTime(sunset);
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
    } else if (timeType == "noon") {
        m_timeType = TimeTypeNoon;
    } else if (timeType == "dawn") {
        m_timeType = TimeTypeDawn;
    } else if (timeType == "sunset") {
        m_timeType = TimeTypeSunset;
    }
}

Alarm::TimeType Alarm::timeType() const
{
    return m_timeType;
}

QDateTime Alarm::calculateOffsetTime(const QDateTime &dateTime) const
{
    QDateTime offsetTime = QDateTime(dateTime);
    offsetTime.time().addSecs(m_offset * 60);
    return offsetTime;
}

bool Alarm::checkDayOfWeek(const QDateTime &dateTime)
{
    QDateTime offsetTime = calculateOffsetTime(dateTime);

    switch (offsetTime.date().dayOfWeek()) {
    case Qt::Monday:
        return monday();
        break;
    case Qt::Tuesday:
        return tuesday();
        break;
    case Qt::Wednesday:
        return wednesday();
        break;
    case Qt::Thursday:
        return thursday();
        break;
    case Qt::Friday:
        return friday();
        break;
    case Qt::Saturday:
        return saturday();
        break;
    case Qt::Sunday:
        return sunday();
        break;
    default:
        return false;
    }
}

bool Alarm::checkHour(const QDateTime &dateTime)
{
    QDateTime offsetTime = calculateOffsetTime(dateTime);

    if (offsetTime.time().hour() != m_hours) {
        return false;
    }
    return true;
}

bool Alarm::checkMinute(const QDateTime &dateTime)
{
    QDateTime offsetTime = calculateOffsetTime(dateTime);

    if (offsetTime.time().minute() != m_minutes) {
        return false;
    }
    return true;
}

bool Alarm::checkTimeTypes(const QDateTime &dateTime)
{
    bool checkOk = false;
    switch (m_timeType) {
    case TimeTypeTime:
        qCWarning(dcDateTime) << name() << "wrong time type";
        checkOk = false;
        break;
    case TimeTypeDusk:
        if (m_duskOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "match dusk with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeSunrise:
        if (m_sunriseOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "match sunrise with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeNoon:
        if (m_noonOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "match noon with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeDawn:
        if (m_dawnOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "match dawn with offset" << m_offset;
            checkOk = true;
        }
        break;
    case TimeTypeSunset:
        if (m_sunsetOffset == dateTime) {
            qCDebug(dcDateTime) << name() << "match sunset with offset" << m_offset;
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
    qCDebug(dcDateTime) << name() << "validate" << dateTime.toString() << "...";

    if (!checkDayOfWeek(dateTime))
        return;

    // check if should use the given time
    if (m_timeType == TimeTypeTime) {
        if (!checkHour(dateTime))
            return;

        if (!checkMinute(dateTime))
            return;

        qCDebug(dcDateTime) << name() << "match time" << QTime(hours(), minutes()).toString("hh:mm") << "with offset" << m_offset;
        emit alarm();
    }
}

void Alarm::validateTimes(const QDateTime &dateTime)
{
    if (m_timeType == TimeTypeTime)
        return;

    if (!checkTimeTypes(dateTime))
        return;

    emit alarm();
}
