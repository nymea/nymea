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

#ifndef UPNPDISCOVERYREPLY_H
#define UPNPDISCOVERYREPLY_H

#include <QObject>

#include "upnpdevicedescriptor.h"

class UpnpDiscoveryReply : public QObject
{
    Q_OBJECT

    friend class UpnpDiscovery;

public:
    enum UpnpDiscoveryReplyError {
        UpnpDiscoveryReplyErrorNoError,
        UpnpDiscoveryReplyErrorNotAvailable,
        UpnpDiscoveryReplyErrorNotEnabled,
        UpnpDiscoveryReplyErrorResourceBusy
    };
    Q_ENUM(UpnpDiscoveryReplyError)

    QString searchTarget() const;
    QString userAgent() const;

    UpnpDiscoveryReplyError error() const;
    bool isFinished() const;

    QList<UpnpDeviceDescriptor> deviceDescriptors() const;

private:
    explicit UpnpDiscoveryReply(const QString &searchTarget, const QString &userAgent, QObject *parent = nullptr);

    QString m_searchTarget;
    QString m_userAgent;

    QList<UpnpDeviceDescriptor> m_deviceDescriptors;
    UpnpDiscoveryReplyError m_error = UpnpDiscoveryReplyErrorNoError;
    bool m_finished = false;

    // Methods for UpnpDiscovery
    void setDeviceDescriptors(const QList<UpnpDeviceDescriptor> &deviceDescriptors);
    void setError(const UpnpDiscoveryReplyError &error);
    void setFinished();

signals:
    void finished();
    void errorOccured(const UpnpDiscoveryReplyError &error);

};

#endif // UPNPDISCOVERYREPLY_H
