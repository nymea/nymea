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
#include "jsontypes.h"

class HttpReply;
class HttpRequest;

namespace guhserver {

class DevicesResource : public QObject
{
    Q_OBJECT
public:
    explicit DevicesResource(QObject *parent = 0);

    HttpReply proccessDeviceRequest(const HttpRequest &request, const QStringList &urlTokens);

private:
    HttpReply getConfiguredDevices();
    HttpReply getConfiguredDevice(Device *device);
    HttpReply getDeviceStateValues(Device *device);
    HttpReply getDeviceStateValue(Device *device, const StateTypeId &stateTypeId);

    HttpReply removeDevice(Device *device);

signals:

public slots:

};

}

#endif // DEVICESRESOURCE_H
