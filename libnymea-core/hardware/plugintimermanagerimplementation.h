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

#ifndef PLUGINTIMERIMPLEMENTATION_H
#define PLUGINTIMERIMPLEMENTATION_H

#include <QTimer>
#include <QObject>
#include <QPointer>

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

}

#endif // PLUGINTIMERIMPLEMENTATION_H
