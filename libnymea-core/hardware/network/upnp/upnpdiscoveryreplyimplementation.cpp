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

#include "upnpdiscoveryreplyimplementation.h"

#include <QTimer>

namespace nymeaserver {

UpnpDiscoveryReplyImplementation::UpnpDiscoveryReplyImplementation(const QString &searchTarget, const QString &userAgent, QObject *parent) :
    UpnpDiscoveryReply(parent),
    m_searchTarget(searchTarget),
    m_userAgent(userAgent)
{

}

QString UpnpDiscoveryReplyImplementation::searchTarget() const
{
    return m_searchTarget;
}

QString UpnpDiscoveryReplyImplementation::userAgent() const
{
    return m_userAgent;
}

UpnpDiscoveryReplyImplementation::UpnpDiscoveryReplyError UpnpDiscoveryReplyImplementation::error() const
{
    return m_error;
}

bool UpnpDiscoveryReplyImplementation::isFinished() const
{
    return m_finished;
}

QList<UpnpDeviceDescriptor> UpnpDiscoveryReplyImplementation::deviceDescriptors() const
{
    return m_deviceDescriptors;
}


void UpnpDiscoveryReplyImplementation::setDeviceDescriptors(const QList<UpnpDeviceDescriptor> &deviceDescriptors)
{
    m_deviceDescriptors = deviceDescriptors;
}

void UpnpDiscoveryReplyImplementation::setError(const UpnpDiscoveryReplyImplementation::UpnpDiscoveryReplyError &error)
{
    m_error = error;
    if (m_error != UpnpDiscoveryReplyErrorNoError) {
        emit errorOccurred(m_error);
    }
}

void UpnpDiscoveryReplyImplementation::setFinished()
{
    m_finished = true;
    // Note: this makes sure the finished signal will be processed in the next event loop
    QTimer::singleShot(0, this, &UpnpDiscoveryReplyImplementation::finished);
}

}
