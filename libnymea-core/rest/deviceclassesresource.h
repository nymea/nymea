/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICECLASSESRESOURCE_H
#define DEVICECLASSESRESOURCE_H

#include <QObject>
#include <QHash>

#include "jsonrpc/jsontypes.h"
#include "restresource.h"
#include "httpreply.h"


namespace nymeaserver {

class HttpRequest;

class DeviceClassesResource : public RestResource
{
    Q_OBJECT
public:
    explicit DeviceClassesResource(QObject *parent = 0);

    QString name() const override;

    HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) override;

private:
    mutable QHash<DeviceClassId, QPointer<HttpReply>> m_discoverRequests;

    DeviceClass m_deviceClass;

    // Process method
    HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens) override;

    // Get methods
    HttpReply *getDeviceClasses(const VendorId &vendorId);
    HttpReply *getDeviceClass();

    HttpReply *getActionTypes();
    HttpReply *getActionType(const ActionTypeId &actionTypeId);

    HttpReply *getStateTypes();
    HttpReply *getStateType(const StateTypeId &stateTypeId);

    HttpReply *getEventTypes();
    HttpReply *getEventType(const EventTypeId &eventTypeId);

    HttpReply *getDiscoverdDevices(const ParamList &discoveryParams);

private slots:
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);


};

}

#endif // DEVICECLASSESRESOURCE_H
