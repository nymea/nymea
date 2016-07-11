/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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
    \class guhserver::TimeManager
    \brief Describes the centralized time manager of guh.

    \ingroup rules
    \inmodule core
*/

/*! \fn void guhserver::TimeManager::tick()
    Represents the central time tick. Will be emitted every second.
*/

/*! \fn void guhserver::TimeManager::dateTimeChanged(const QDateTime &dateTime);
    Will be emitted when the \a dateTime has changed.
*/

#include "timemanager.h"
#include "guhcore.h"
#include "loggingcategories.h"

namespace guhserver {

/*! Constructs a new \l{TimeManager} with the given \a timeZone and \a parent. */
TimeManager::TimeManager(const QByteArray &timeZone, QObject *parent) :
    QObject(parent)
{
    m_dateTime = QDateTime::currentDateTimeUtc();
    m_dateTime.setTimeSpec(Qt::UTC);

    setTimeZone(timeZone);

    m_guhTimer = new QTimer(this);
    m_guhTimer->setInterval(1000);
    m_guhTimer->setSingleShot(false);

    connect(m_guhTimer, &QTimer::timeout, this, &TimeManager::guhTimeout);

    m_guhTimer->start();
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

QList<QByteArray> TimeManager::availableTimeZones() const
{
    return QTimeZone::availableTimeZoneIds();
}

#ifdef TESTING_ENABLED
void TimeManager::stopTimer()
{
    // Stop clock (used for testing)
    m_guhTimer->stop();
}

void TimeManager::setTime(const QDateTime &dateTime)
{
    // This method will only be called for testing to set the guhIO intern time
    emit tick();
    emit dateTimeChanged(dateTime.toTimeZone(m_timeZone));
}
#endif

void TimeManager::guhTimeout()
{
    // tick for deviceManager
    emit tick();

    // Minute based guh time
    QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    if (m_dateTime.time().minute() != currentDateTime.toTimeZone(m_timeZone).time().minute()) {
        m_dateTime = currentDateTime;
        emit dateTimeChanged(m_dateTime.toTimeZone(m_timeZone));
    }
}

}
