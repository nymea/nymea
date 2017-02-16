/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2016 Michael Zanetti <michael_zanetti@gmx.net>           *
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
    \page networkdetector.html
    \title Network Detector
    \brief Plugin to monitor devices in the local network.

    \ingroup plugins
    \ingroup guh-plugins


    This plugin allows to find and monitor network devices in your local network by using the hostname of the devices.

    \underline{NOTE}: the application \c nmap has to be installed and guh has to run as root.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/networkdetector/devicepluginnetworkdetector.json
*/


#include "devicepluginnetworkdetector.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>
#include <QNetworkInterface>

DevicePluginNetworkDetector::DevicePluginNetworkDetector():
    m_discoveryProcess(0),
    m_scanProcess(0),
    m_aboutToQuit(false)
{

}

DevicePluginNetworkDetector::~DevicePluginNetworkDetector()
{
    // Stop running processes
    m_aboutToQuit = true;

    if (m_scanProcess && m_scanProcess->state() == QProcess::Running) {
        qCDebug(dcNetworkDetector()) << "Kill running scan process";
        m_scanProcess->kill();
        m_scanProcess->waitForFinished(5000);
    }

    if (m_discoveryProcess && m_discoveryProcess->state() == QProcess::Running) {
        qCDebug(dcNetworkDetector()) << "Kill running discovery process";
        m_discoveryProcess->terminate();
        m_discoveryProcess->waitForFinished(5000);
    }
}

DeviceManager::DeviceSetupStatus DevicePluginNetworkDetector::setupDevice(Device *device)
{
    qCDebug(dcNetworkDetector()) << "Setup" << device->name() << device->params();
    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginNetworkDetector::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)


    if (deviceClassId != networkDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    if (m_discoveryProcess) {
        qCWarning(dcNetworkDetector()) << "Network discovery already running";
        return DeviceManager::DeviceErrorDeviceInUse;
    }

    m_discoveryProcess = startScanProcesses();
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::HardwareResources DevicePluginNetworkDetector::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

void DevicePluginNetworkDetector::guhTimer()
{
    if (!myDevices().isEmpty() && !m_scanProcess)
        m_scanProcess = startScanProcesses();

}

QProcess * DevicePluginNetworkDetector::startScanProcesses()
{
    QStringList targets = getDefaultTargets();
    qCDebug(dcNetworkDetector()) << "Start network discovery" << targets;
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));

    QStringList arguments;
    arguments << "-R" << "-oX" << "-" << "-v" << "--stats-every" << "1" << "-sn";
    arguments << targets;

    process->start(QStringLiteral("nmap"), arguments);
    return process;
}


QStringList DevicePluginNetworkDetector::getDefaultTargets()
{
    QStringList targets;
    foreach (const QHostAddress &interface, QNetworkInterface::allAddresses()) {
        if (!interface.isLoopback() && interface.scopeId().isEmpty()) {
            QPair<QHostAddress, int> pair = QHostAddress::parseSubnet(interface.toString() + "/24");
            targets << QString("%1/%2").arg(pair.first.toString()).arg(pair.second);
        }
    }
    return targets;
}

QList<Host> DevicePluginNetworkDetector::parseProcessOutput(const QByteArray &processData)
{
    m_reader.clear();
    m_reader.addData(processData);

    QList<Host> hosts;

    while (!m_reader.atEnd() && !m_reader.hasError()) {

        QXmlStreamReader::TokenType token = m_reader.readNext();
        if(token == QXmlStreamReader::StartDocument)
            continue;

        if(token == QXmlStreamReader::StartElement && m_reader.name() == "host") {
            Host host = parseHost();
            if (host.isValid()) {
                hosts.append(host);
            }
        }
    }
    return hosts;
}

Host DevicePluginNetworkDetector::parseHost()
{
    if (!m_reader.isStartElement() || m_reader.name() != "host")
        return Host();

    QString address; QString hostName; QString status;
    while(!(m_reader.tokenType() == QXmlStreamReader::EndElement && m_reader.name() == "host")){

        m_reader.readNext();

        if (m_reader.isStartElement() && m_reader.name() == "hostname") {
            QString name = m_reader.attributes().value("name").toString();
            if (!name.isEmpty())
                hostName = name;

            m_reader.readNext();
        }

        if (m_reader.name() == "address") {
            QString addr = m_reader.attributes().value("addr").toString();
            if (!addr.isEmpty())
                address = addr;
        }

        if (m_reader.name() == "status") {
            QString state = m_reader.attributes().value("state").toString();
            if (!state.isEmpty())
                status = state;
        }
    }

    return Host(hostName, address, (status == "up" ? true : false));
}

void DevicePluginNetworkDetector::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess*>(sender());

    // If the process was killed because guhd is shutting down...we dont't care any more about the result
    if (m_aboutToQuit)
        return;

    // Discovery
    if (process == m_discoveryProcess) {

        qCDebug(dcNetworkDetector()) << "Discovery process finished";

        process->deleteLater();
        m_discoveryProcess = 0;

        QList<DeviceDescriptor> deviceDescriptors;
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            qCWarning(dcNetworkDetector) << "Network scan error:" << process->readAllStandardError();
            emit devicesDiscovered(networkDeviceClassId, deviceDescriptors);
            return;
        }

        QByteArray outputData = process->readAllStandardOutput();
        foreach (const Host &host, parseProcessOutput(outputData)) {
            DeviceDescriptor descriptor(networkDeviceClassId, host.hostName(), host.adderss());
            descriptor.setParams( ParamList() << Param(hostnameParamTypeId, host.hostName()));
            deviceDescriptors.append(descriptor);
        }

        emit devicesDiscovered(networkDeviceClassId, deviceDescriptors);

    } else if (process == m_scanProcess) {
        // Scan
        qCDebug(dcNetworkDetector()) << "Network scan process finished";

        process->deleteLater();
        m_scanProcess = 0;

        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            qCWarning(dcNetworkDetector) << "Network scan error:" << process->readAllStandardError();
            return;
        }

        if (myDevices().isEmpty()) {
            process->deleteLater();
            return;
        }

        QStringList upHosts;
        QByteArray outputData = process->readAllStandardOutput();
        foreach (const Host &host, parseProcessOutput(outputData)) {
            if (host.isValid() && host.reachable())
                upHosts.append(host.hostName());

        }

        foreach (Device *device, myDevices()) {
            if (upHosts.contains(device->paramValue(hostnameParamTypeId).toString())) {
                device->setStateValue(inRangeStateTypeId, true);
            } else {
                device->setStateValue(inRangeStateTypeId, false);
            }
        }
    }
}
