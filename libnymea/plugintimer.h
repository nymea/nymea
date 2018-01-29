/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

#ifndef PLUGINTIMER_H
#define PLUGINTIMER_H

#include <QTimer>
#include <QObject>
#include <QPointer>

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

    virtual PluginTimer *registerTimer(int seconds = 60) = 0;
    virtual void unregisterTimer(PluginTimer *timer = nullptr) = 0;
};

#endif // PLUGINTIMER_H
