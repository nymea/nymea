/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPLUGINGPIO_H
#define DEVICEPLUGINGPIO_H

#include "hardware/gpio.h"
#include "hardware/gpiomonitor.h"
#include "gpiodescriptor.h"
#include "hardware/gpiomonitor.h"
#include "plugin/deviceplugin.h"

class DevicePluginGpio : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugingpio.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginGpio();

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    void deviceRemoved(Device *device) override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void postSetupDevice(Device *device);

private:
    QHash<Gpio *, Device *> m_gpioDevices;
    QHash<GpioMonitor *, Device *> m_monitorDevices;

    QHash<int, Gpio *> m_raspberryPiGpios;
    QHash<int, GpioMonitor *> m_raspberryPiGpioMoniors;

    QHash<int, Gpio *> m_beagleboneBlackGpios;
    QHash<int, GpioMonitor *> m_beagleboneBlackGpioMoniors;

    QList<GpioDescriptor> raspberryPiGpioDescriptors();
    QList<GpioDescriptor> beagleboneBlackGpioDescriptors();

private slots:
    void onGpioValueChanged(const bool &value);

};

#endif // DEVICEPLUGINGPIO_H
