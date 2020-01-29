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
TimeManager::TimeManager(QObject *parent) :
    QObject(parent)
{
    m_timerId = startTimer(1000, Qt::VeryCoarseTimer);
}

/*! Returns the current dateTime of this \l{TimeManager}. */
QDateTime TimeManager::currentDateTime() const
{
    return QDateTime::currentDateTime().addSecs(m_overrideDifference);
}

/*! Stop the time.
 *
 * \note This method should only be used in tests.
*/
void TimeManager::stopTimer()
{
    qCWarning(dcTimeManager()) << "TimeManager timer stopped. You should only see this in tests.";
    // Stop clock (used for testing)
    killTimer(m_timerId);
}

/*! Set the current time of this TimeManager to the given \a dateTime.
 *
 * \note This method should only be used in tests.
*/
void TimeManager::setTime(const QDateTime &dateTime)
{
    qCWarning(dcTimeManager()) << "TimeManager time changed" << dateTime.toString("dd.MM.yyyy hh:mm:ss") << "You should only see this in tests.";
    m_overrideDifference = QDateTime::currentDateTime().secsTo(dateTime);
    // This method will only be called for testing to set the internal time
    emit dateTimeChanged(dateTime);
}

void TimeManager::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)

    emit tick();

    // Minute based nymea time
    QDateTime now = QDateTime::currentDateTime();
    if (m_lastEvent.time().minute() != now.time().minute()) {
        m_lastEvent = now;
        emit dateTimeChanged(now.addSecs(m_overrideDifference));
    }
}

}
