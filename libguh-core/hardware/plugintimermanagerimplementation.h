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

#ifndef PLUGINTIMERIMPLEMENTATION_H
#define PLUGINTIMERIMPLEMENTATION_H

#include <QTimer>
#include <QObject>
#include <QPointer>

#include "plugintimer.h"

namespace guhserver {

class PluginTimerImplementation : public PluginTimer
{
    Q_OBJECT

    friend class PluginTimerManagerImplementation;

public:
    explicit PluginTimerImplementation(int intervall, QObject *parent = nullptr);

    int interval() const;
    int currentTick() const;
    bool running() const;

signals:
    void timeout();
    void currentTickChanged(const int &currentTick);
    void runningChanged(const bool &running);
    void pausedChanged(const bool &paused);

private:
    int m_interval;
    int m_currentTick = 0;

    bool m_paused = false;
    bool m_running = true;

    void setRunning(const bool &running);
    void setPaused(const bool &paused);
    void setCurrentTick(const int &tick);

    void tick();

public slots:
    void reset();
    void start();
    void stop();
    void pause();
    void resume();

};

class PluginTimerManagerImplementation : public PluginTimerManager
{
    Q_OBJECT

    friend class HardwareManagerImplementation;

public:
    explicit PluginTimerManagerImplementation(QObject *parent = nullptr);

    PluginTimer *registerTimer(int seconds = 60);
    void unregisterTimer(PluginTimer *timer = nullptr);


private:
    QList<QPointer<PluginTimerImplementation> > m_timers;
    void timeTick();

public slots:
    bool enable();
    bool disable();

};

}

#endif // PLUGINTIMERIMPLEMENTATION_H
