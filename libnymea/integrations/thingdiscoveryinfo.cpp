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

#include "thingdiscoveryinfo.h"
#include "thingmanager.h"

#include <QTimer>

Q_DECLARE_LOGGING_CATEGORY(dcIntegrations)

ThingDiscoveryInfo::ThingDiscoveryInfo(const ThingClassId &thingClassId, const ParamList &params, ThingManager *thingManager, quint32 timeout)
    : QObject(thingManager)
    , m_thingClassId(thingClassId)
    , m_params(params)
    , m_thingManager(thingManager)
{
    connect(this, &ThingDiscoveryInfo::finished, this, &ThingDiscoveryInfo::deleteLater, Qt::QueuedConnection);

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

ThingClassId ThingDiscoveryInfo::thingClassId() const
{
    return m_thingClassId;
}

ParamList ThingDiscoveryInfo::params() const
{
    return m_params;
}

bool ThingDiscoveryInfo::isFinished() const
{
    return m_finished;
}

Thing::ThingError ThingDiscoveryInfo::status() const
{
    return m_status;
}

void ThingDiscoveryInfo::addThingDescriptor(const ThingDescriptor &thingDescriptor)
{
    m_thingDescriptors.append(thingDescriptor);
}

void ThingDiscoveryInfo::addThingDescriptors(const ThingDescriptors &thingDescriptors)
{
    m_thingDescriptors.append(thingDescriptors);
}

ThingDescriptors ThingDiscoveryInfo::thingDescriptors() const
{
    return m_thingDescriptors;
}

QString ThingDiscoveryInfo::displayMessage() const
{
    return m_displayMessage;
}

QString ThingDiscoveryInfo::translatedDisplayMessage(const QLocale &locale)
{
    if (!m_thingManager) {
        return m_displayMessage;
    }
    ThingClass thingClass = m_thingManager->findThingClass(m_thingClassId);
    return m_thingManager->translate(thingClass.pluginId(), m_displayMessage.toUtf8(), locale);
}

void ThingDiscoveryInfo::finish(Thing::ThingError status, const QString &displayMessage)
{
    if (m_finished) {
        qCWarning(dcIntegrations()) << "ThingDiscoveryInfo::finish() called on an already finished object.";
        return;
    }
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
