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

#ifndef PLUGINTIMER_H
#define PLUGINTIMER_H

#include <QObject>
#include <QPointer>
#include <QTimer>

#include "hardwareresource.h"

class PluginTimer : public QObject
{
    Q_OBJECT

public:
    PluginTimer(QObject *parent = nullptr);
    virtual ~PluginTimer() = default;

    virtual int interval() const = 0;
    virtual int currentTick() const = 0;
    virtual bool running() const = 0;

signals:
    void timeout();

    void currentTickChanged(const int &currentTick);
    void runningChanged(const bool &running);
    void pausedChanged(const bool &paused);

public slots:
    virtual void reset() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
};

class PluginTimerManager : public HardwareResource
{
    Q_OBJECT

public:
    PluginTimerManager(QObject *parent = nullptr);
    virtual ~PluginTimerManager() = default;

    Q_INVOKABLE virtual PluginTimer *registerTimer(int seconds = 60) = 0;
    Q_INVOKABLE virtual void unregisterTimer(PluginTimer *timer = nullptr) = 0;
};

#endif // PLUGINTIMER_H
