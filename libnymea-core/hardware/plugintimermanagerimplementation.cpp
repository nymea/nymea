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

#include "plugintimermanagerimplementation.h"
#include "loggingcategories.h"
#include "nymeacore.h"

namespace nymeaserver {

PluginTimerImplementation::PluginTimerImplementation(int interval, QObject *parent) :
    PluginTimer(parent),
    m_interval(interval)
{
    connect(NymeaCore::instance()->timeManager(), &TimeManager::tick, this, &PluginTimerImplementation::tick);
}

int PluginTimerImplementation::interval() const
{
    return m_interval;
}

int PluginTimerImplementation::currentTick() const
{
    return m_currentTick;
}

bool PluginTimerImplementation::running() const
{
    return m_running;
}

void PluginTimerImplementation::setRunning(bool running)
{
    if (m_running != running) {
        m_running = running;
        emit runningChanged(m_running);
    }
}

void PluginTimerImplementation::setPaused(bool paused)
{
    if (m_paused != paused) {
        m_paused = paused;
        emit pausedChanged(m_paused);
    }
}

void PluginTimerImplementation::setCurrentTick(int tick)
{
    if (m_currentTick != tick) {
        m_currentTick = tick;
        emit currentTickChanged(m_currentTick);
    }
}

void PluginTimerImplementation::tick()
{
    if (m_paused)
        return;

    if (!m_running)
        return;

    setCurrentTick(m_currentTick += 1);

    if (m_currentTick >= m_interval) {
        emit timeout();
        reset();
    }
}

void PluginTimerImplementation::reset()
{
    setCurrentTick(0);
}

void PluginTimerImplementation::start()
{
    setPaused(false);
    setRunning(true);
}

void PluginTimerImplementation::stop()
{
    setPaused(false);
    setRunning(false);
}

void PluginTimerImplementation::pause()
{
    m_paused = true;
}

void PluginTimerImplementation::resume()
{
    m_paused = false;
}


PluginTimerManagerImplementation::PluginTimerManagerImplementation(QObject *parent) :
    PluginTimerManager(parent)
{
    m_available = true;
    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

PluginTimer *PluginTimerManagerImplementation::registerTimer(int seconds)
{
    QPointer<PluginTimerImplementation> pluginTimer = new PluginTimerImplementation(seconds, this);
    qCDebug(dcHardware()) << "Register timer" << pluginTimer->interval();

    // TODO: schedule timer for load balancing

    //    // Get min/max timer interval
    //    PluginTimer *shortestTimer = nullptr;
    //    PluginTimer *longestTimer = nullptr;
    //    foreach (PluginTimer *timer, m_timers) {
    //        if (!shortestTimer && !longestTimer) {
    //            shortestTimer = timer;
    //            longestTimer = timer;
    //            continue;
    //        }

    //        if (timer->interval() < shortestTimer->interval()) {
    //            shortestTimer = timer;
    //            continue;
    //        }

    //        if (timer->interval() > longestTimer->interval()) {
    //            longestTimer = timer;
    //            continue;
    //        }
    //    }

    m_timers.append(pluginTimer);
    return pluginTimer.data();
}

void PluginTimerManagerImplementation::unregisterTimer(PluginTimer *timer)
{
    QPointer<PluginTimer> timerPointer(timer);
    if (timerPointer.isNull()) {
        qCWarning(dcHardware()) << name() << "Cannot unregister timer. Looks like the timer is already unregistered.";
        return;
    }

    qCDebug(dcHardware()) << "Unregister timer" << timer->interval();

    foreach (QPointer<PluginTimerImplementation> tPointer, m_timers) {
        if (timerPointer.data() == tPointer.data()) {
            m_timers.removeAll(tPointer);
            tPointer->deleteLater();
        }
    }
}

bool PluginTimerManagerImplementation::available() const
{
    return m_available;
}

bool PluginTimerManagerImplementation::enabled() const
{
    return m_enabled;
}

void PluginTimerManagerImplementation::timeTick()
{
    // If timer resource is not enabled do nothing
    if (!enabled()) {
        return;
    }

    foreach (PluginTimerImplementation *timer, m_timers) {
        timer->tick();
    }
}

void PluginTimerManagerImplementation::setEnabled(bool enabled)
{
    if (enabled == m_enabled) {
        qCDebug(dcHardware()) << "TimerManager already" << (enabled ? "enabled": "disabled");
        return;
    }

    m_enabled = enabled;
    emit enabledChanged(enabled);

    foreach (QPointer<PluginTimerImplementation> timer, m_timers) {
        timer->setPaused(enabled);
    }
}

bool PluginTimerManagerImplementation::enable()
{
    if (!available())
        return false;

    setEnabled(true);
    return true;
}

bool PluginTimerManagerImplementation::disable()
{
    if (!available())
        return false;

    setEnabled(false);
    return true;
}

}


