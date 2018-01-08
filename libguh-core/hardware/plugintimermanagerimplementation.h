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

    int interval() const override;
    int currentTick() const override;
    bool running() const override;

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
    void reset() override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;

};

class PluginTimerManagerImplementation : public PluginTimerManager
{
    Q_OBJECT

    friend class HardwareManagerImplementation;

public:
    explicit PluginTimerManagerImplementation(QObject *parent = nullptr);

    PluginTimer *registerTimer(int seconds = 60) override;
    void unregisterTimer(PluginTimer *timer = nullptr) override;

    bool available() const override;
    bool enabled() const override;

private:
    QList<QPointer<PluginTimerImplementation> > m_timers;
    void timeTick();

protected:
    void setEnabled(bool enabled) override;

public slots:
    bool enable();
    bool disable();

private:
    bool m_available = false;
    bool m_enabled = false;

};

}

#endif // PLUGINTIMERIMPLEMENTATION_H
