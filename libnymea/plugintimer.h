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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

    Q_INVOKABLE virtual PluginTimer *registerTimer(int seconds = 60) = 0;
    Q_INVOKABLE virtual void unregisterTimer(PluginTimer *timer = nullptr) = 0;
};

#endif // PLUGINTIMER_H
