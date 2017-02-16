/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

/*!
    \page gpioplugin.html
    \title GPIO Plugin
    \brief Plugin to controll gpios on different boards.

    \ingroup plugins
    \ingroup guh-plugins-maker

    \chapter Raspberry Pi 2

    \image Raspberry-Pi-2-GPIO.png "Raspberry Pi 2 GPIOs"

    \chapter Beaglebone Black

    \image Beaglebone-Black-GPIO.png "Beaglebone Black GPIOs"

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
    if (device->deviceClassId() == gpioSwitchRpiDeviceClassId || device->deviceClassId() == gpioSwitchBbbDeviceClassId) {
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

        if (device->deviceClassId() == gpioSwitchRpiDeviceClassId)
            m_raspberryPiGpios.insert(gpio->gpioNumber(), gpio);

        if (device->deviceClassId() == gpioSwitchBbbDeviceClassId)
            m_beagleboneBlackGpios.insert(gpio->gpioNumber(), gpio);

        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == gpioButtonRpiDeviceClassId || device->deviceClassId() == gpioButtonBbbDeviceClassId) {
        GpioMonitor *monior = new GpioMonitor(device->paramValue(gpioParamTypeId).toInt(), this);

        if (!monior->enable()) {
            qCWarning(dcGpioController()) << "Could not enable gpio monitor for device" << device->name();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        connect(monior, &GpioMonitor::valueChanged, this, &DevicePluginGpio::onGpioValueChanged);

        m_monitorDevices.insert(monior, device);

        if (device->deviceClassId() == gpioSwitchRpiDeviceClassId)
            m_raspberryPiGpioMoniors.insert(monior->gpio()->gpioNumber(), monior);

        if (device->deviceClassId() == gpioSwitchBbbDeviceClassId)
            m_beagleboneBlackGpioMoniors.insert(monior->gpio()->gpioNumber(), monior);

        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginGpio::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    // Check if GPIOs are available on this platform
    if (!Gpio::isAvailable()) {
        qCWarning(dcGpioController()) << "There are no GPIOs on this plattform";
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
        return DeviceManager::DeviceErrorAsync;
    }

    if (deviceClass.vendorId() == beagleboneBlackVendorId) {

        // Create the list of available gpios
        QList<DeviceDescriptor> deviceDescriptors;

        QList<GpioDescriptor> gpioDescriptors = beagleboneBlackGpioDescriptors();
        for (int i = 0; i < gpioDescriptors.count(); i++) {
            const GpioDescriptor gpioDescriptor = gpioDescriptors.at(i);

            // Offer only gpios which arn't in use already
            if (m_beagleboneBlackGpios.keys().contains(gpioDescriptor.gpio()))
                continue;

            if (m_beagleboneBlackGpioMoniors.keys().contains(gpioDescriptor.gpio()))
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
        return DeviceManager::DeviceErrorAsync;
    }

    return DeviceManager::DeviceErrorVendorNotFound;
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

        if (m_beagleboneBlackGpios.values().contains(gpio))
            m_beagleboneBlackGpios.remove(gpio->gpioNumber());

        delete gpio;
    }

    if (m_monitorDevices.values().contains(device)) {
        GpioMonitor *monitor = m_monitorDevices.key(device);
        if (!monitor)
            return;

        m_monitorDevices.remove(monitor);

        if (m_raspberryPiGpioMoniors.values().contains(monitor))
            m_raspberryPiGpios.remove(monitor->gpio()->gpioNumber());

        if (m_beagleboneBlackGpioMoniors.values().contains(monitor))
            m_beagleboneBlackGpioMoniors.remove(monitor->gpio()->gpioNumber());

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

    if (deviceClass.vendorId() == beagleboneBlackVendorId)
        gpio = m_beagleboneBlackGpios.value(device->paramValue(gpioParamTypeId).toInt());

    // Check if gpio was found
    if (!gpio) {
        qCWarning(dcGpioController()) << "Could not find gpio for executing action on" << device->name();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // GPIO Switch power action
    if (action.actionTypeId() == powerValueActionTypeId) {
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

        // Set the current state
        device->setStateValue(powerValueStateTypeId, action.param(powerValueStateParamTypeId).value());

        return DeviceManager::DeviceErrorNoError;
    }

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginGpio::postSetupDevice(Device *device)
{
    if (device->deviceClassId() == gpioSwitchRpiDeviceClassId || device->deviceClassId() == gpioSwitchBbbDeviceClassId) {
        Gpio *gpio = m_gpioDevices.key(device);
        if (!gpio)
            return;

        gpio->setValue(Gpio::ValueLow);
        device->setStateValue(powerValueStateTypeId, false);
    }

    if (device->deviceClassId() == gpioButtonRpiDeviceClassId || device->deviceClassId() == gpioButtonBbbDeviceClassId) {
        GpioMonitor *monitor = m_monitorDevices.key(device);
        if (!monitor)
            return;

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

QList<GpioDescriptor> DevicePluginGpio::beagleboneBlackGpioDescriptors()
{
    // Note: https://www.mathworks.com/help/examples/beaglebone_product/beaglebone_black_gpio_pinmap.png
    QList<GpioDescriptor> gpioDescriptors;
    gpioDescriptors << GpioDescriptor(2, 22, "P9");
    gpioDescriptors << GpioDescriptor(3, 21, "P9");
    gpioDescriptors << GpioDescriptor(4, 18, "P9 - I2C1_SDA");
    gpioDescriptors << GpioDescriptor(5, 17, "P9 - I2C1_SCL");
    gpioDescriptors << GpioDescriptor(7, 42, "P9");
    gpioDescriptors << GpioDescriptor(12, 20, "P9 - I2C2_SDA");
    gpioDescriptors << GpioDescriptor(13, 19, "P9 - I2C2_SCL");
    gpioDescriptors << GpioDescriptor(14, 26, "P9");
    gpioDescriptors << GpioDescriptor(15, 24, "P9");
    gpioDescriptors << GpioDescriptor(20, 41, "P9");
    gpioDescriptors << GpioDescriptor(30, 11, "P9");
    gpioDescriptors << GpioDescriptor(31, 13, "P9");
    gpioDescriptors << GpioDescriptor(48, 15, "P9");
    gpioDescriptors << GpioDescriptor(49, 23, "P9");
    gpioDescriptors << GpioDescriptor(50, 14, "P9");
    gpioDescriptors << GpioDescriptor(51, 16, "P9");
    gpioDescriptors << GpioDescriptor(60, 12, "P9");
    gpioDescriptors << GpioDescriptor(117, 25, "P9");
    gpioDescriptors << GpioDescriptor(120, 31, "P9");
    gpioDescriptors << GpioDescriptor(121, 29, "P9");
    gpioDescriptors << GpioDescriptor(122, 30, "P9");
    gpioDescriptors << GpioDescriptor(123, 28, "P9");

    gpioDescriptors << GpioDescriptor(8, 35, "P8 - LCD_DATA12");
    gpioDescriptors << GpioDescriptor(9, 33, "P8 - LCD_DATA13");
    gpioDescriptors << GpioDescriptor(10, 31, "P8 - LCD_DATA14");
    gpioDescriptors << GpioDescriptor(11, 32, "P8 - LCD_DATA15");
    gpioDescriptors << GpioDescriptor(22, 19, "P8");
    gpioDescriptors << GpioDescriptor(23, 13, "P8");
    gpioDescriptors << GpioDescriptor(26, 14, "P8");
    gpioDescriptors << GpioDescriptor(27, 17, "P8");
    gpioDescriptors << GpioDescriptor(32, 25, "P8 - MMC1-DAT0");
    gpioDescriptors << GpioDescriptor(33, 24, "P8 - MMC1_DAT1");
    gpioDescriptors << GpioDescriptor(34, 5, "P8 - MMC1_DAT2");
    gpioDescriptors << GpioDescriptor(35, 6, "P8 - MMC1_DAT3");
    gpioDescriptors << GpioDescriptor(36, 23, "P8 - MMC1-DAT4");
    gpioDescriptors << GpioDescriptor(37, 22, "P8 - MMC1_DAT5");
    gpioDescriptors << GpioDescriptor(38, 3, "P8 - MMC1_DAT6");
    gpioDescriptors << GpioDescriptor(39, 4, "P8 - MMC1_DAT7");
    gpioDescriptors << GpioDescriptor(44, 12, "P8");
    gpioDescriptors << GpioDescriptor(45, 11, "P8");
    gpioDescriptors << GpioDescriptor(46, 16, "P8");
    gpioDescriptors << GpioDescriptor(47, 15, "P8");
    gpioDescriptors << GpioDescriptor(61, 26, "P8");
    gpioDescriptors << GpioDescriptor(62, 21, "P8 - MMC1-CLK");
    gpioDescriptors << GpioDescriptor(63, 20, "P8 - MMC1_CMD");
    gpioDescriptors << GpioDescriptor(65, 18, "P8");
    gpioDescriptors << GpioDescriptor(66, 7, "P8");
    gpioDescriptors << GpioDescriptor(67, 8, "P8");
    gpioDescriptors << GpioDescriptor(68, 10, "P8");
    gpioDescriptors << GpioDescriptor(69, 9, "P8");
    gpioDescriptors << GpioDescriptor(70, 45, "P8 - LCD_DATA0");
    gpioDescriptors << GpioDescriptor(71, 46, "P8 - LCD_DATA1");
    gpioDescriptors << GpioDescriptor(72, 43, "P8 - LCD_DATA2");
    gpioDescriptors << GpioDescriptor(73, 44, "P8 - LCD_DATA3");
    gpioDescriptors << GpioDescriptor(74, 41, "P8 - LCD_DATA4");
    gpioDescriptors << GpioDescriptor(75, 42, "P8 - LCD_DATA5");
    gpioDescriptors << GpioDescriptor(76, 39, "P8 - LCD_DATA6");
    gpioDescriptors << GpioDescriptor(77, 40, "P8 - LCD_DATA7");
    gpioDescriptors << GpioDescriptor(78, 37, "P8 - LCD_DATA8");
    gpioDescriptors << GpioDescriptor(79, 38, "P8 - LCD_DATA9");
    gpioDescriptors << GpioDescriptor(80, 36, "P8 - LCD_DATA10");
    gpioDescriptors << GpioDescriptor(81, 34, "P8 - LCD_DATA11");
    gpioDescriptors << GpioDescriptor(86, 27, "P8 - LCD_VSYNC");
    gpioDescriptors << GpioDescriptor(87, 29, "P8 - LCD_HSYNC");
    gpioDescriptors << GpioDescriptor(88, 28, "P8 - LCD_PCLK");
    gpioDescriptors << GpioDescriptor(89, 30, "P8 - LCD_AC_BIAS_E");
    return gpioDescriptors;
}

void DevicePluginGpio::onGpioValueChanged(const bool &value)
{
    GpioMonitor *monitor = static_cast<GpioMonitor *>(sender());

    Device *device = m_monitorDevices.value(monitor);
    if (!device)
        return;

    device->setStateValue(pressedStateTypeId, value);
}

