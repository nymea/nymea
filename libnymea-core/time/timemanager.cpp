/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::TimeManager
    \brief Describes the centralized time manager of nymea.

    \ingroup rules
    \inmodule core
*/

/*! \fn void nymeaserver::TimeManager::tick()
    Represents the central time tick. Will be emitted every second.
*/

/*! \fn void nymeaserver::TimeManager::dateTimeChanged(const QDateTime &dateTime);
    Will be emitted when the \a dateTime has changed.
*/

#include "timemanager.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l{TimeManager} with the given \a timeZone and \a parent. */
TimeManager::TimeManager(const QByteArray &timeZone, QObject *parent) :
    QObject(parent)
{
    m_dateTime = QDateTime::currentDateTimeUtc();
    m_dateTime.setTimeSpec(Qt::UTC);

    setTimeZone(timeZone);

    m_nymeaTimer = new QTimer(this);
    m_nymeaTimer->setInterval(1000);
    m_nymeaTimer->setSingleShot(false);

    connect(m_nymeaTimer, &QTimer::timeout, this, &TimeManager::nymeaTimeout);

    m_nymeaTimer->start();
}

/*! Returns the time zone of this \l{TimeManager}. */
QByteArray TimeManager::timeZone() const
{
    return m_timeZone.id();
}

/*! Sets the \a timeZone of this \l{TimeManager}. Allowed values according to the \l{http://www.iana.org/time-zones}{IANA database}.
 *  Returns false if the given timezone is not valid. */
bool TimeManager::setTimeZone(const QByteArray &timeZone)
{
    if (!QTimeZone(timeZone).isValid()) {
        qCWarning(dcTimeManager()) << "Invalid time zone" << timeZone;
        qCWarning(dcTimeManager()) << "Using system time zone" << QTimeZone::systemTimeZoneId();
        m_timeZone = QTimeZone(QTimeZone::systemTimeZoneId());
        emit dateTimeChanged(currentDateTime());
        return false;
    }

    qCDebug(dcTimeManager()) << "Set time zone" << timeZone;
    m_timeZone = QTimeZone(timeZone);
    qCDebug(dcTimeManager()) << "UTC" << m_dateTime.toString("dd.MM.yyyy hh:mm:ss");
    qCDebug(dcTimeManager) << "Zone time" << currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
    emit dateTimeChanged(currentDateTime());
    return true;
}

/*! Returns the current dateTime of this \l{TimeManager}. */
QDateTime TimeManager::currentDateTime() const
{
    return QDateTime::currentDateTimeUtc().toTimeZone(m_timeZone);
}

/*! Returns the current time of this \l{TimeManager}. */
QTime TimeManager::currentTime() const
{
    return QDateTime::currentDateTimeUtc().toTimeZone(m_timeZone).time();
}

/*! Returns the current date of this \l{TimeManager}. */
QDate TimeManager::currentDate() const
{
    return QDateTime::currentDateTimeUtc().toTimeZone(m_timeZone).date();
}

/*! Returns a list of available time zones on this system. */
QList<QByteArray> TimeManager::availableTimeZones() const
{
    return QTimeZone::availableTimeZoneIds();
}

void TimeManager::stopTimer()
{
    qCWarning(dcTimeManager()) << "TimeManager timer stopped. You should only see this in tests.";
    // Stop clock (used for testing)
    m_nymeaTimer->stop();
}

void TimeManager::setTime(const QDateTime &dateTime)
{
    qCWarning(dcTimeManager()) << "TimeManager time changed" << dateTime.toString("dd.MM.yyyy hh:mm:ss") << "You should only see this in tests.";
    // This method will only be called for testing to set the internal time
    emit tick();
    emit dateTimeChanged(dateTime);
}

void TimeManager::nymeaTimeout()
{
    // tick for deviceManager
    emit tick();

    // Minute based nymea time
    QDateTime utcDateTime = QDateTime::currentDateTimeUtc();
    if (m_dateTime.time().minute() != utcDateTime.toTimeZone(m_timeZone).time().minute()) {
        m_dateTime = utcDateTime;
        emit dateTimeChanged(currentDateTime());
    }
}

}
