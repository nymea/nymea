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

#ifndef DEVICEACTIONINFO_H
#define DEVICEACTIONINFO_H

#include <QObject>

#include "device.h"
#include "typeutils.h"
#include "types/action.h"

class DeviceManager;

class DeviceActionInfo : public QObject
{
    Q_OBJECT
public:
    explicit DeviceActionInfo(Device *device, const Action &action, DeviceManager *parent, quint32 timeout = 0);

    Device* device() const;
    Action action() const;

    bool isFinished() const;

    Device::DeviceError status() const;

    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale);

signals:
    void finished();
    void aborted();

public slots:
    void finish(Device::DeviceError status, const QString &displayMessage = QString());

private:
    Device *m_device = nullptr;
    Action m_action;

    bool m_finished = false;
    Device::DeviceError m_status = Device::DeviceErrorNoError;
    QString m_displayMessage;

    DeviceManager *m_deviceManager = nullptr;
};

#endif // DEVICEACTIONINFO_H
