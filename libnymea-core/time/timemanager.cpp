/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
 * \note This method should only be used in tests and when shutting down the system.
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
