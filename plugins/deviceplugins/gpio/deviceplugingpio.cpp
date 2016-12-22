/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

/*!
    \page gpioplugin.html
    \title GPIO Plugin
    \brief Plugin to controll gpios on different boards.

    \ingroup plugins
    \ingroup guh-plugins-maker

    \chapter Raspberry Pi 2

    \image Raspberry-Pi-2-GPIO.png "Raspberry Pi 2 GPIOs"

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/gpio/deviceplugingpio.json
*/


#include "deviceplugingpio.h"
#include "types/param.h"
#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

DevicePluginGpio::DevicePluginGpio()
{

}

DeviceManager::DeviceSetupStatus DevicePluginGpio::setupDevice(Device *device)
{
    qCDebug(dcGpioController()) << "Setup" << device->name() << device->params();

    // Check if GPIOs are available on this platform
    if (!Gpio::isAvailable()) {
        qCWarning(dcGpioController()) << "There are ou GPIOs on this plattform";
        return DeviceManager::DeviceSetupStatusFailure;
    }

    // GPIO Switch
    if (device->deviceClassId() == gpioSwitchDeviceClassId) {
        // Create and configure gpio
        Gpio *gpio = new Gpio(device->paramValue(gpioParamTypeId).toInt(), this);

        if (!gpio->exportGpio()) {
            qCWarning(dcGpioController()) << "Could not export gpio for device" << device->name();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        if (!gpio->setDirection(Gpio::DirectionOutput)) {
            qCWarning(dcGpioController()) << "Could not configure output gpio for device" << device->name();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        if (!gpio->setValue(Gpio::ValueLow)) {
            qCWarning(dcGpioController()) << "Could not set gpio  value for device" << device->name();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        m_gpioDevices.insert(gpio, device);
        m_raspberryPiGpios.insert(gpio->gpioNumber(), gpio);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == gpioSwitchDeviceClassId) {
        GpioMonitor *monior = new GpioMonitor(device->paramValue(gpioParamTypeId).toInt(), this);
        if (!monior->enable()) {
            qCWarning(dcGpioController()) << "Could not enable gpio monitor for device" << device->name();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        connect(monior, &GpioMonitor::valueChanged, this, &DevicePluginGpio::onGpioValueChanged);

        m_monitorDevices.insert(monior, device);
        m_raspberryPiGpioMoniors.insert(monior->gpio()->gpioNumber(), monior);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginGpio::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    // Check if GPIOs are available on this platform
    if (!Gpio::isAvailable()) {
        qCWarning(dcGpioController()) << "There are ou GPIOs on this plattform";
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // Check which board / gpio configuration
    const DeviceClass deviceClass = deviceManager()->findDeviceClass(deviceClassId);
    if (deviceClass.vendorId() == raspberryPiVendorId) {

        // Create the list of available gpios
        QList<DeviceDescriptor> deviceDescriptors;

        QList<GpioDescriptor> gpioDescriptors = raspberryPiGpioDescriptors();
        for (int i = 0; i < gpioDescriptors.count(); i++) {
            const GpioDescriptor gpioDescriptor = gpioDescriptors.at(i);

            // Offer only gpios which arn't in use already
            if (m_raspberryPiGpios.keys().contains(gpioDescriptor.gpio()))
                continue;

            if (m_raspberryPiGpioMoniors.keys().contains(gpioDescriptor.gpio()))
                continue;

            QString description;
            if (gpioDescriptor.description().isEmpty()) {
                description = QString("Pin %1").arg(gpioDescriptor.pin());
            } else {
                description = QString("Pin %1 | %2").arg(gpioDescriptor.pin()).arg(gpioDescriptor.description());
            }

            DeviceDescriptor descriptor(deviceClassId, QString("GPIO %1").arg(gpioDescriptor.gpio()), description);
            ParamList parameters;
            parameters.append(Param(gpioParamTypeId, gpioDescriptor.gpio()));
            parameters.append(Param(pinParamTypeId, gpioDescriptor.pin()));
            parameters.append(Param(descriptionParamTypeId, gpioDescriptor.description()));
            descriptor.setParams(parameters);

            deviceDescriptors.append(descriptor);
        }

        emit devicesDiscovered(deviceClassId, deviceDescriptors);
    }

    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::HardwareResources DevicePluginGpio::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginGpio::deviceRemoved(Device *device)
{
    if (m_gpioDevices.values().contains(device)) {
        Gpio *gpio = m_gpioDevices.key(device);
        if (!gpio)
            return;

        m_gpioDevices.remove(gpio);

        if (m_raspberryPiGpios.values().contains(gpio))
            m_raspberryPiGpios.remove(gpio->gpioNumber());

        delete gpio;
    }

    if (m_monitorDevices.values().contains(device)) {
        GpioMonitor *monitor = m_monitorDevices.key(device);
        if (!monitor)
            return;

        m_monitorDevices.remove(monitor);

        if (m_raspberryPiGpioMoniors.values().contains(monitor))
            m_raspberryPiGpios.remove(monitor->gpio()->gpioNumber());

        delete monitor;
    }

}

DeviceManager::DeviceError DevicePluginGpio::executeAction(Device *device, const Action &action)
{
    // Get the gpio
    const DeviceClass deviceClass = deviceManager()->findDeviceClass(device->deviceClassId());
    Gpio *gpio = Q_NULLPTR;
    // Find the gpio in the corresponding hash
    if (deviceClass.vendorId() == raspberryPiVendorId)
        gpio = m_raspberryPiGpios.value(device->paramValue(gpioParamTypeId).toInt());

    // Check if gpio was found
    if (!gpio) {
        qCWarning(dcGpioController()) << "Could not find gpio for executing action on" << device->name();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // GPIO Switch power action
    if (device->deviceClassId() == gpioSwitchDeviceClassId && action.actionTypeId() == powerValueActionTypeId) {
        bool success = false;
        if (action.param(powerValueStateParamTypeId).value().toBool()) {
            success = gpio->setValue(Gpio::ValueHigh);
        } else {
            success = gpio->setValue(Gpio::ValueLow);
        }

        if (!success) {
            qCWarning(dcGpioController()) << "Could not set gpio value while execute action on" << device->name();
            return DeviceManager::DeviceErrorHardwareFailure;
        }

        return DeviceManager::DeviceErrorNoError;
    }

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginGpio::postSetupDevice(Device *device)
{
    if (device->deviceClassId() == gpioSwitchDeviceClassId) {
        Gpio *gpio = m_gpioDevices.key(device);
        device->setStateValue(powerValueStateTypeId, (bool)gpio->value());
    }

    if (device->deviceClassId() == gpioButtonDeviceClassId) {
        GpioMonitor *monitor = m_monitorDevices.key(device);
        device->setStateValue(pressedStateTypeId, monitor->value());
    }
}

QList<GpioDescriptor> DevicePluginGpio::raspberryPiGpioDescriptors()
{
    // Note: http://www.raspberrypi-spy.co.uk/wp-content/uploads/2012/06/Raspberry-Pi-GPIO-Layout-Model-B-Plus-rotated-2700x900.png
    QList<GpioDescriptor> gpioDescriptors;
    gpioDescriptors << GpioDescriptor(2, 3, "SDA1_I2C");
    gpioDescriptors << GpioDescriptor(3, 5, "SCL1_I2C");
    gpioDescriptors << GpioDescriptor(4, 7);
    gpioDescriptors << GpioDescriptor(5, 29);
    gpioDescriptors << GpioDescriptor(6, 31);
    gpioDescriptors << GpioDescriptor(7, 26, "SPI0_CE1_N");
    gpioDescriptors << GpioDescriptor(8, 24, "SPI0_CE0_N");
    gpioDescriptors << GpioDescriptor(9, 21, "SPI0_MISO");
    gpioDescriptors << GpioDescriptor(10, 19, "SPI0_MOSI");
    gpioDescriptors << GpioDescriptor(11, 23, "SPI0_SCLK");
    gpioDescriptors << GpioDescriptor(12, 32);
    gpioDescriptors << GpioDescriptor(13, 33);
    gpioDescriptors << GpioDescriptor(14, 8, "UART0_TXD");
    gpioDescriptors << GpioDescriptor(15, 10, "UART0_RXD");
    gpioDescriptors << GpioDescriptor(16, 36);
    gpioDescriptors << GpioDescriptor(17, 11);
    gpioDescriptors << GpioDescriptor(18, 12, "PCM_CLK");
    gpioDescriptors << GpioDescriptor(19, 35);
    gpioDescriptors << GpioDescriptor(20, 38);
    gpioDescriptors << GpioDescriptor(21, 40);
    gpioDescriptors << GpioDescriptor(22, 15);
    gpioDescriptors << GpioDescriptor(23, 16);
    gpioDescriptors << GpioDescriptor(24, 18);
    gpioDescriptors << GpioDescriptor(25, 22);
    gpioDescriptors << GpioDescriptor(26, 37);
    gpioDescriptors << GpioDescriptor(27, 13);
    return gpioDescriptors;
}

void DevicePluginGpio::onGpioValueChanged(const bool &value)
{
    GpioMonitor *monitor = static_cast<GpioMonitor *>(sender());

    // Get device and set state value
    if (m_raspberryPiGpioMoniors.values().contains(monitor)) {
        Device *device = m_monitorDevices.value(monitor);
        if (!device)
            return;

        device->setStateValue(pressedStateTypeId, value);
    }

}

