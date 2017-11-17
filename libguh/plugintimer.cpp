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

PluginTimer::PluginTimer(int intervall, QObject *parent) :
    HardwareResource(HardwareResource::TypeTimer, "Plugin timer", parent),
    m_intervall(intervall)
{
    // FIXME: the timer should be able to emit timerEvents with different resolutions
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(m_intervall);

    connect(m_timer, &QTimer::timeout, this, &PluginTimer::timerEvent);
    setAvailable(true);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

bool PluginTimer::enable()
{
    if (!available())
        return false;

    m_timer->start();
    setEnabled(true);
    return true;
}

bool PluginTimer::disable()
{
    if (!available())
        return false;

    m_timer->stop();
    setEnabled(false);
    return true;
}
