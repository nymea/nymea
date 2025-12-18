// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PLUGINTIMERIMPLEMENTATION_H
#define PLUGINTIMERIMPLEMENTATION_H

#include <QObject>
#include <QPointer>
#include <QTimer>

#include "plugintimer.h"

namespace nymeaserver {

class PluginTimerImplementation : public PluginTimer
{
    Q_OBJECT

    friend class PluginTimerManagerImplementation;

public:
    explicit PluginTimerImplementation(int interval, QObject *parent = nullptr);

    int interval() const override;
    int currentTick() const override;
    bool running() const override;

private:
    int m_interval;
    int m_currentTick = 0;

    bool m_paused = false;
    bool m_running = true;

    void setRunning(bool running);
    void setPaused(bool paused);
    void setCurrentTick(int tick);

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

} // namespace nymeaserver

#endif // PLUGINTIMERIMPLEMENTATION_H
