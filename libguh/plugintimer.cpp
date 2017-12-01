/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "plugintimer.h"
#include "loggingcategories.h"

int PluginTimer::interval() const
{
    return m_interval;
}

void PluginTimer::pause()
{
    m_paused = true;
}

void PluginTimer::resume()
{
    m_paused = false;
}

int PluginTimer::currentTick() const
{
    return m_currentTick;
}

bool PluginTimer::running() const
{
    return m_running;
}

PluginTimer::PluginTimer(int intervall, QObject *parent) :
    QObject(parent),
    m_interval(intervall)
{

}

void PluginTimer::setRunning(const bool &running)
{
    if (m_running != running) {
        m_running = running;
        emit runningChanged(m_running);
    }
}

void PluginTimer::setPaused(const bool &paused)
{
    if (m_paused != paused) {
        m_paused = paused;
        emit pausedChanged(m_paused);
    }
}

void PluginTimer::setCurrentTick(const int &tick)
{
    if (m_currentTick != tick) {
        m_currentTick = tick;
        emit currentTickChanged(m_currentTick);
    }
}

void PluginTimer::tick()
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

void PluginTimer::reset()
{
    setCurrentTick(0);
}

void PluginTimer::start()
{
    setPaused(false);
    setRunning(true);
}

void PluginTimer::stop()
{
    setPaused(false);
    setRunning(false);
}


PluginTimer *PluginTimerManager::registerTimer(int seconds)
{
    QPointer<PluginTimer> pluginTimer = new PluginTimer(seconds, this);
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

void PluginTimerManager::unregisterTimer(PluginTimer *timer)
{
    QPointer<PluginTimer> timerPointer(timer);
    if (timerPointer.isNull()) {
        qCWarning(dcHardware()) << name() << "Cannot unregister timer. Looks like the timer is already unregistered.";
        return;
    }

    qCDebug(dcHardware()) << "Unregister timer" << timer->interval();

    foreach (QPointer<PluginTimer> tPointer, m_timers) {
        if (timerPointer.data() == tPointer.data()) {
            m_timers.removeAll(tPointer);
            tPointer->deleteLater();
        }
    }
}

PluginTimerManager::PluginTimerManager(QObject *parent) :
    HardwareResource(HardwareResource::TypeTimer, "Plugin timer manager", parent)
{
    setAvailable(true);
    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

void PluginTimerManager::timeTick()
{
    // If timer resource is not enabled do nothing
    if (!enabled()) {
        return;
    }

    foreach (PluginTimer *timer, m_timers) {
        timer->tick();
    }
}

bool PluginTimerManager::enable()
{
    if (!available())
        return false;

    setEnabled(true);
    return true;
}

bool PluginTimerManager::disable()
{
    if (!available())
        return false;

    setEnabled(false);
    return true;
}


