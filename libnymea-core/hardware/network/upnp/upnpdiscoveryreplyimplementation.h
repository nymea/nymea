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

#ifndef UPNPDISCOVERYREPLYIMPLEMENTATION_H
#define UPNPDISCOVERYREPLYIMPLEMENTATION_H

#include <QObject>

#include "network/upnp/upnpdiscoveryreply.h"
#include "network/upnp/upnpdevicedescriptor.h"

namespace nymeaserver {

class UpnpDiscoveryReplyImplementation : public UpnpDiscoveryReply
{
    Q_OBJECT

    friend class UpnpDiscoveryImplementation;

public:
    explicit UpnpDiscoveryReplyImplementation(const QString &searchTarget, const QString &userAgent, QObject *parent = nullptr);

    QString searchTarget() const override;
    QString userAgent() const override;

    UpnpDiscoveryReplyError error() const override;
    bool isFinished() const override;

    QList<UpnpDeviceDescriptor> deviceDescriptors() const override;

private:
    QString m_searchTarget;
    QString m_userAgent;

    QList<UpnpDeviceDescriptor> m_deviceDescriptors;
    UpnpDiscoveryReplyError m_error = UpnpDiscoveryReplyErrorNoError;
    bool m_finished = false;

    // Methods for UpnpDiscoveryImplementation
    void setDeviceDescriptors(const QList<UpnpDeviceDescriptor> &deviceDescriptors);
    void setError(const UpnpDiscoveryReplyError &error);
    void setFinished();

};

}

#endif // UPNPDISCOVERYREPLYIMPLEMENTATION_H
