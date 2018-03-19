/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

#ifndef UPNPDISCOVERYREPLY_H
#define UPNPDISCOVERYREPLY_H

#include <QObject>

#include "upnpdevicedescriptor.h"

class UpnpDiscoveryReply : public QObject
{
    Q_OBJECT

public:
    enum UpnpDiscoveryReplyError {
        UpnpDiscoveryReplyErrorNoError,
        UpnpDiscoveryReplyErrorNotAvailable,
        UpnpDiscoveryReplyErrorNotEnabled,
        UpnpDiscoveryReplyErrorResourceBusy
    };
    Q_ENUM(UpnpDiscoveryReplyError)

    explicit UpnpDiscoveryReply(QObject *parent = nullptr);
    virtual ~UpnpDiscoveryReply() = default;

    virtual QString searchTarget() const = 0;
    virtual QString userAgent() const = 0;

    virtual UpnpDiscoveryReplyError error() const = 0;
    virtual bool isFinished() const = 0;

    virtual QList<UpnpDeviceDescriptor> deviceDescriptors() const = 0;

signals:
    void finished();
    void errorOccurred(const UpnpDiscoveryReplyError &error);

};

#endif // UPNPDISCOVERYREPLY_H
