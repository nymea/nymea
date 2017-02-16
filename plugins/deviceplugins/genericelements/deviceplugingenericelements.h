/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPLUGINGENERICELEMENTS_H
#define DEVICEPLUGINGENERICELEMENTS_H

#include "plugin/deviceplugin.h"

class DevicePluginGenericElements : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugingenericelements.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginGenericElements();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;


public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

};

#endif // DEVICEPLUGINGENERICELEMENTS_H
