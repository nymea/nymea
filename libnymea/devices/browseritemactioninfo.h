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

#ifndef BROWSERITEMACTIONINFO_H
#define BROWSERITEMACTIONINFO_H

#include <QObject>

#include "device.h"
#include "types/browseritemaction.h"

class BrowserItemActionInfo : public QObject
{
    Q_OBJECT
public:
    explicit BrowserItemActionInfo(Device *device, const BrowserItemAction &browserItemAction, QObject *parent, quint32 timeout = 0);

    Device *device() const;
    BrowserItemAction browserItemAction() const;

    bool isFinished() const;

    Device::DeviceError status() const;

signals:
    void finished();
    void aborted();

public slots:
    void finish(Device::DeviceError status);

private:
    Device *m_device = nullptr;
    BrowserItemAction m_browserItemAction;

    bool m_finished = false;
    Device::DeviceError m_status = Device::DeviceErrorNoError;
};

#endif // BROWSERITEMACTIONINFO_H
