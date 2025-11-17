// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "serialportmonitor.h"
#include "loggingcategories.h"

namespace nymeaserver {

NYMEA_LOGGING_CATEGORY(dcSerialPortMonitor, "SerialPortMonitor")

SerialPortMonitor::SerialPortMonitor(QObject *parent) : QObject(parent)
{
    // Read initially all tty devices
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        m_serialPorts.insert(serialPortInfo.systemLocation(), SerialPort(serialPortInfo));
    }

#ifdef WITH_UDEV
    // Init udev
    m_udev = udev_new();
    if (!m_udev) {
        qCWarning(dcSerialPortMonitor()) << "Could not initialize udev for the adapter monitor";
        return;
    }

    // Create udev monitor
    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor) {
        qCWarning(dcSerialPortMonitor()) << "Could not initialize udev monitor.";
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // Set monitor filter to tty subsystem
    if (udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "tty", nullptr) < 0) {
        qCWarning(dcSerialPortMonitor()) << "Could not set subsystem device type filter to tty.";
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // Enable the monitor
    if (udev_monitor_enable_receiving(m_monitor) < 0) {
        qCWarning(dcSerialPortMonitor()) << "Could not enable udev monitor.";
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
        udev_unref(m_udev);
        m_udev = nullptr;
        return;
    }

    // Create socket notifier for read
    int socketDescriptor = udev_monitor_get_fd(m_monitor);
    m_notifier = new QSocketNotifier(socketDescriptor, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, [this, socketDescriptor](int socket){
        // Make sure the socket matches
        if (socketDescriptor != socket) {
            qCWarning(dcSerialPortMonitor()) << "Socket handles do not match. socket != socketdescriptor";
            return;
        }

        // Create udev device
        udev_device *device = udev_monitor_receive_device(m_monitor);
        if (!device) {
            qCWarning(dcSerialPortMonitor()) << "Got socket sotification but could not read device information.";
            return;
        }

        QString actionString = QString::fromLatin1(udev_device_get_action(device));
        QString systemPath = QString::fromLatin1(udev_device_get_property_value(device,"DEVNAME"));
        QString manufacturerString = QString::fromLatin1(udev_device_get_property_value(device,"ID_VENDOR_ENC"));
        QString descriptionString = QString::fromLatin1(udev_device_get_property_value(device,"ID_MODEL_ENC"));
        QString serialNumberString = QString::fromLatin1(udev_device_get_property_value(device, "ID_SERIAL_SHORT"));

        // Clean udev device
        udev_device_unref(device);

        // Make sure we know the action
        if (actionString.isEmpty())
            return;

        if (actionString == "add") {
            qCDebug(dcSerialPortMonitor()) << "[+]" << systemPath << serialNumberString << manufacturerString << descriptionString;
            if (!m_serialPorts.contains(systemPath)) {
                // Get the serial port info and add it internally
                foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
                    if (serialPortInfo.systemLocation() == systemPath) {
                        addSerialPortInternally(SerialPort(serialPortInfo));
                    }
                }

            }
        }

        if (actionString == "remove") {
            qCDebug(dcSerialPortMonitor()) << "[-]" << systemPath << serialNumberString << manufacturerString << descriptionString;
            if (m_serialPorts.contains(systemPath)) {
                SerialPort serialPort = m_serialPorts.take(systemPath);
                qCDebug(dcSerialPortMonitor()) << "Removed" << serialPort.systemLocation();
                emit serialPortRemoved(serialPort);
            }
        }
    });

    qCDebug(dcSerialPortMonitor()) << "Serial port monitor enabled successfully";
    foreach (const SerialPort &serialPort, m_serialPorts) {
        qCDebug(dcSerialPortMonitor()) << "-" << serialPort.systemLocation() << serialPort.description();
    }
    m_notifier->setEnabled(true);
#else
    m_timer = new QTimer(this);
    m_timer->setInterval(5000);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, [=](){
        QStringList availablePorts;
        // Add a new adapter if not in the list already
        foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
            availablePorts.append(serialPortInfo.systemLocation());
            if (!m_serialPorts.keys().contains(serialPortInfo.systemLocation())) {
                qCDebug(dcSerialPortMonitor()) << "[+]" << serialPortInfo.systemLocation() << serialPortInfo.manufacturer() << serialPortInfo.description();
                addSerialPortInternally(SerialPort(serialPortInfo));
            }
        }
        // Remove adapters no longer available
        foreach (const QString &systemLocation, m_serialPorts.keys()) {
            if (!availablePorts.contains(systemLocation)) {
                SerialPort serialPortInfo = m_serialPorts.take(systemLocation);
                qCDebug(dcSerialPortMonitor()) << "[-]" << serialPortInfo.systemLocation() << serialPortInfo.manufacturer() << serialPortInfo.description();
                emit serialPortRemoved(serialPortInfo);
            }
        }
    });
    m_timer->start();

#endif
}

SerialPorts SerialPortMonitor::serialPorts() const
{
    return m_serialPorts.values();
}

bool SerialPortMonitor::serialPortAvailable(const QString &systemLocation) const
{
    return m_serialPorts.keys().contains(systemLocation);
}

void SerialPortMonitor::addSerialPortInternally(const SerialPort &serialPort)
{
    if (m_serialPorts.keys().contains(serialPort.systemLocation())) {
        qCWarning(dcSerialPortMonitor()) << "Tried to add serial port but the port has alrady been added.";
        return;
    }

    m_serialPorts.insert(serialPort.systemLocation(), serialPort);
    emit serialPortAdded(serialPort);
}

}
