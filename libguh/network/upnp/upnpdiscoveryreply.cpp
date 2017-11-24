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

#include "upnpdiscoveryreply.h"

#include <QTimer>

QString UpnpDiscoveryReply::searchTarget() const
{
    return m_searchTarget;
}

QString UpnpDiscoveryReply::userAgent() const
{
    return m_userAgent;
}

UpnpDiscoveryReply::UpnpDiscoveryReplyError UpnpDiscoveryReply::error() const
{
    return m_error;
}

bool UpnpDiscoveryReply::isFinished() const
{
    return m_finished;
}

QList<UpnpDeviceDescriptor> UpnpDiscoveryReply::deviceDescriptors() const
{
    return m_deviceDescriptors;
}

UpnpDiscoveryReply::UpnpDiscoveryReply(const QString &searchTarget, const QString &userAgent, QObject *parent) :
    QObject(parent),
    m_searchTarget(searchTarget),
    m_userAgent(userAgent)
{

}

void UpnpDiscoveryReply::setDeviceDescriptors(const QList<UpnpDeviceDescriptor> &deviceDescriptors)
{
    m_deviceDescriptors = deviceDescriptors;
}

void UpnpDiscoveryReply::setError(const UpnpDiscoveryReply::UpnpDiscoveryReplyError &error)
{
    m_error = error;
    if (m_error != UpnpDiscoveryReplyErrorNoError) {
        emit errorOccured(m_error);
    }
}

void UpnpDiscoveryReply::setFinished()
{
    m_finished = true;
    // Note: this makes sure the finished signal will be processed in the next event loop
    QTimer::singleShot(0, this, &UpnpDiscoveryReply::finished);
}
