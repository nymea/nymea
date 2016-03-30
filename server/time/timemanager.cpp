/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
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

/*! \fn void guhserver::TimeManager::dateChanged(const QDate &currentDate);
    Will be emitted when the \a currentDate has changed.
*/

/*! \fn void guhserver::TimeManager::timeChanged(const QTime &currentTime);
    Will be emitted when the \a currentTime has changed.
*/

#include "timemanager.h"
#include "loggingcategories.h"

namespace guhserver {

/*! Constructs a new \l{TimeManager} with the given \a timeZone and \a parent. */
TimeManager::TimeManager(const QByteArray &timeZone, QObject *parent) :
    QObject(parent)
{
    m_dateTime = QDateTime::currentDateTimeUtc();
    m_dateTime.setTimeSpec(Qt::UTC);
    qCDebug(dcTimeManager()) << "UTC" << m_dateTime.toString("dd.MM.yyyy hh:mm:ss");

    setTimeZone(timeZone);
    qCDebug(dcTimeManager) << m_dateTime.toTimeZone(m_timeZone).toString("dd.MM.yyyy hh:mm:ss");

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

/*! Sets the \a timeZone of this \l{TimeManager}. */
void TimeManager::setTimeZone(const QByteArray &timeZone)
{
    if (!QTimeZone(timeZone).isValid()) {
        qCWarning(dcTimeManager()) << "Invalid time zone" << timeZone;
        qCWarning(dcTimeManager()) << "Using default system timezone" << QTimeZone::systemTimeZoneId();
        m_timeZone = QTimeZone(QTimeZone::systemTimeZoneId());
    } else {
        qCDebug(dcTimeManager()) << "Set time zone" << timeZone;
        m_timeZone = QTimeZone(timeZone);
    }
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

void TimeManager::guhTimeout()
{
    // tick for deviceManager
    emit tick();

    QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    //qCDebug(dcTimeManager) << "Time changed" << currentDateTime.toTimeZone(m_timeZone).time().toString("hh:mm:ss");

    // Minute based guh time
    if (m_dateTime.time().minute() != currentDateTime.toTimeZone(m_timeZone).time().minute()) {
        m_dateTime = currentDateTime;
        emit timeChanged(m_dateTime.toTimeZone(m_timeZone).time());
    }

    // check if day changed
    if (m_dateTime.date() != currentDateTime.toTimeZone(m_timeZone).date()) {
        emit dateChanged(m_dateTime.toTimeZone(m_timeZone).date());
    }
}

}
