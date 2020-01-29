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
