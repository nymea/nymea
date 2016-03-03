/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
    \page wifidetector.html
    \title WiFi Detector

    \ingroup plugins
    \ingroup network

    This plugin allows to find and monitor network devices in your local network by using the MAC address.

    \underline{NOTE}: the application \c nmap has to be installed and guh has to run as root.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/wifidetector/devicepluginwifidetector.json
*/


#include "devicepluginwifidetector.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>
#include <QNetworkInterface>

DevicePluginWifiDetector::DevicePluginWifiDetector()
{
}

DeviceManager::DeviceSetupStatus DevicePluginWifiDetector::setupDevice(Device *device)
{
    qCDebug(dcWifiDetector) << "Setup" << device->name() << device->params();
    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginWifiDetector::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    m_deviceDescriptors.clear();

    if (deviceClassId != wifiDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        // not localhost and IPv4
        if (!address.isLoopback() && address.protocol() == QAbstractSocket::IPv4Protocol) {
            QProcess *process = new QProcess(this);
            qCDebug(dcWifiDetector) << "Discover interface" << address.toString();
            connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(discoveryProcessFinished(int,QProcess::ExitStatus)));
            process->start(QStringLiteral("nmap"), QStringList() << "-sP" << QString("%1/24").arg(address.toString()));
            m_discoveryProcesses.append(process);
        }
    }

    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::HardwareResources DevicePluginWifiDetector::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

void DevicePluginWifiDetector::guhTimer()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        // not localhost and IPv4
        if (!address.isLoopback() && address.protocol() == QAbstractSocket::IPv4Protocol) {
            QProcess *process = new QProcess(this);
            connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
            process->start(QStringLiteral("nmap"), QStringList() << "-sP" << QString("%1/24").arg(address.toString()));
        }
    }
}

void DevicePluginWifiDetector::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess*>(sender());

    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        qCWarning(dcWifiDetector) << "Network scan error:" << process->readAllStandardError();
        process->deleteLater();
        return;
    }

    // return if there is no longer any device
    if (myDevices().isEmpty()) {
        process->deleteLater();
        return;
    }

    QStringList foundDevices;
    while(process->canReadLine()) {
        QString result = QString::fromLatin1(process->readLine());
        if (result.startsWith("MAC Address:")) {
            QStringList lineParts = result.split(' ');
            if (lineParts.count() > 3) {
                foundDevices << lineParts.at(2).toLower();
            }
        }
    }

    // check states
    foreach (Device *device, myDevices()) {
        bool found = foundDevices.contains(device->paramValue("mac address").toString().toLower());
        device->setStateValue(inRangeStateTypeId, found);
    }
    process->deleteLater();
}

void DevicePluginWifiDetector::discoveryProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess*>(sender());

    qCDebug(dcWifiDetector) << "Discovery finished";

    if (!m_discoveryProcesses.contains(process))
        return;

    m_discoveryProcesses.removeAll(process);
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        qCWarning(dcWifiDetector) << "Network scan error:" << process->readAllStandardError();
        process->deleteLater();
        return;
    }

    while(process->canReadLine()) {
        QString result = QString::fromLatin1(process->readLine());
        if (result.startsWith("MAC Address:")) {
            QStringList lineParts = result.split(' ');
            if (lineParts.count() > 4) {
                QString macAddress = lineParts.at(2).toLower();
                int index = result.indexOf(lineParts.at(2));
                QString name = result.right(result.length() - index - macAddress.length() - 1).remove(QRegExp("\\(|\\)"));
                qCDebug(dcWifiDetector) << "Found" << name << macAddress.toLower();
                DeviceDescriptor descriptor(wifiDeviceClassId, name, macAddress);
                ParamList params;
                params.append(Param("mac address", macAddress));
                descriptor.setParams(params);
                m_deviceDescriptors.append(descriptor);
            }
        }
    }

    if (m_discoveryProcesses.isEmpty())
        emit devicesDiscovered(wifiDeviceClassId, m_deviceDescriptors);
}
