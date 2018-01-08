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

#ifndef UPNPDISCOVERYREPLYIMPLEMENTATION_H
#define UPNPDISCOVERYREPLYIMPLEMENTATION_H

#include <QObject>

#include "network/upnp/upnpdiscoveryreply.h"
#include "network/upnp/upnpdevicedescriptor.h"

namespace guhserver {

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
