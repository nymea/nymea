/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
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

#ifndef DEVICEDISCOVERYINFO_H
#define DEVICEDISCOVERYINFO_H

#include <QObject>

#include "types/deviceclass.h"
#include "types/param.h"
#include "device.h"
#include "devicedescriptor.h"

class DeviceManager;

class LIBNYMEA_EXPORT DeviceDiscoveryInfo : public QObject
{
    Q_OBJECT
public:
    explicit DeviceDiscoveryInfo(const DeviceClassId &deviceClassId, const ParamList &params, DeviceManager *deviceManager, quint32 timeout = 0);

    DeviceClassId deviceClassId() const;
    ParamList params() const;

    bool isFinished() const;

    Device::DeviceError status() const;

    void addDeviceDescriptor(const DeviceDescriptor &deviceDescriptor);
    void addDeviceDescriptors(const DeviceDescriptors &deviceDescriptors);

    DeviceDescriptors deviceDescriptors() const;

    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale);

public slots:
    void finish(Device::DeviceError status,  const QString &displayMessage = QString());

signals:
    void finished();
    void aborted();

private:
    DeviceClassId m_deviceClassId;
    ParamList m_params;

    bool m_finished = false;
    Device::DeviceError m_status;
    QString m_displayMessage;
    DeviceDescriptors m_deviceDescriptors;

    DeviceManager *m_deviceManager = nullptr;
};

#endif // DEVICEDISCOVERYINFO_H
