/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICESRESOURCE_H
#define DEVICESRESOURCE_H

#include <QObject>
#include <QHash>

#include "jsontypes.h"
#include "restresource.h"
#include "httpreply.h"

namespace guhserver {

class HttpRequest;

class DevicesResource: public RestResource
{
    Q_OBJECT
public:
    explicit DevicesResource(QObject *parent = 0);

    QString name() const override;

    HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) override;

private:
    mutable QHash<ActionId, QPointer<HttpReply> > m_asyncActionExecutions;
    mutable QHash<DeviceId, QPointer<HttpReply> > m_asyncDeviceAdditions;
    mutable QHash<Device *, QPointer<HttpReply> > m_asyncReconfigureDevice;
    mutable QHash<PairingTransactionId, QPointer<HttpReply> > m_asyncPairingRequests;

    Device *m_device;

    // Process method
    HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessOptionsRequest(const HttpRequest &request, const QStringList &urlTokens) override;

    // Get methods
    HttpReply *getConfiguredDevices() const;
    HttpReply *getConfiguredDevice(Device *device) const;
    HttpReply *getDeviceStateValues(Device *device) const;
    HttpReply *getDeviceStateValue(Device *device, const StateTypeId &stateTypeId) const;

    // Delete methods
    HttpReply *removeDevice(Device *device, const QVariantMap &params) const;

    // Post methods
    HttpReply *executeAction(Device *device, const ActionTypeId &actionTypeId, const QByteArray &payload) const;
    HttpReply *addConfiguredDevice(const QByteArray &payload) const;
    HttpReply *editDevice(const QByteArray &payload) const;
    HttpReply *pairDevice(const QByteArray &payload) const;
    HttpReply *confirmPairDevice(const QByteArray &payload) const;

    // Put methods
    HttpReply *reconfigureDevice(Device *device, const QByteArray &payload) const;

private slots:
    void actionExecuted(const ActionId &actionId, DeviceManager::DeviceError status);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceError status);
    void deviceReconfigurationFinished(Device *device, DeviceManager::DeviceError status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceError status, const DeviceId &deviceId);
};

}

#endif // DEVICESRESOURCE_H
