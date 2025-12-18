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

#include "thingactioninfo.h"

#include "thingmanager.h"

#include <QTimer>

Q_DECLARE_LOGGING_CATEGORY(dcIntegrations)

ThingActionInfo::ThingActionInfo(Thing *thing, const Action &action, ThingManager *parent, quint32 timeout)
    : QObject(parent)
    , m_thing(thing)
    , m_action(action)
    , m_thingManager(parent)
{
    connect(this, &ThingActionInfo::finished, this, &ThingActionInfo::deleteLater, Qt::QueuedConnection);

    // TODO 2.0: Create a base class and move this finished() of all Info classes logic handling there
    // That will badly break the ABI, so copy/pasting this for now.
    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            // It can happen that a plugin calls finish() in a slot which normally would be dispatched before the timeout
            // but due to high system load the slot is invoked only after the timeout. This in turn would cause Qt to also queue up
            // this timeout slot and by the time the system processes slots, the plugin comes in first and we'd fire an aborted()
            // signal in the plugin after it called finish(), potentially badly breaking the plugin as a plugin developer would not
            // expect this to happen. So we'll have to verify here that the plugin did not finish() by now before aborting.
            if (!m_finished) {
                emit aborted();
                // Also it could happen the plugin calls finish() *in* the aborted() signal, in which case
                // we don't want to call finish() here now to not print a warning about duplicate finished calls
                if (!m_finished) {
                    finish(Thing::ThingErrorTimeout);
                }
            }
        });
    }
}

Thing *ThingActionInfo::thing() const
{
    return m_thing;
}

Action ThingActionInfo::action() const
{
    return m_action;
}

bool ThingActionInfo::isFinished() const
{
    return m_finished;
}

Thing::ThingError ThingActionInfo::status() const
{
    return m_status;
}

QString ThingActionInfo::displayMessage() const
{
    return m_displayMessage;
}

QString ThingActionInfo::translatedDisplayMessage(const QLocale &locale)
{
    if (!m_thingManager) {
        return m_displayMessage;
    }
    return m_thingManager->translate(m_thing->pluginId(), m_displayMessage.toUtf8(), locale);
}

void ThingActionInfo::finish(Thing::ThingError status, const QString &displayMessage)
{
    if (m_finished) {
        qCWarning(dcIntegrations()) << "ThingActionInfo::finish() called on an already finished object:" << m_thing << m_action.actionTypeId().toString();
        return;
    }
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
