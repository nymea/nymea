/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "devicepluginwifidetector.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>

DeviceClassId detectorId = DeviceClassId("bd216356-f1ec-4324-9785-6982d2174e17");
StateTypeId inRangeStateTypeId = StateTypeId("cb43e1b5-4f61-4538-bfa2-c33055c542cf");

DevicePluginWifiDetector::DevicePluginWifiDetector()
{
}

//QList<DeviceClass> DevicePluginWifiDetector::supportedDevices() const
//{
//    QList<DeviceClass> ret;

//    DeviceClass deviceClassWifiDetector(pluginId(), supportedVendors().first().id(), detectorId);
//    deviceClassWifiDetector.setName("WiFi Device");
    
//    QList<ParamType> detectorParams;
//    ParamType macParam("mac", QVariant::String);
//    detectorParams.append(macParam);

//    deviceClassWifiDetector.setParamTypes(detectorParams);

//    QList<StateType> detectorStates;

//    StateType inRangeState(inRangeStateTypeId);
//    inRangeState.setName("inRange");
//    inRangeState.setType(QVariant::Bool);
//    inRangeState.setDefaultValue(false);
//    detectorStates.append(inRangeState);

//    deviceClassWifiDetector.setStateTypes(detectorStates);

//    ret.append(deviceClassWifiDetector);

//    return ret;
//}

DeviceManager::HardwareResources DevicePluginWifiDetector::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

void DevicePluginWifiDetector::guhTimer()
{
    QProcess *p = new QProcess(this);
    connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
    p->start(QStringLiteral("sudo"), QStringList() << "nmap" << "-sP" << "10.10.10.0/24");
}

void DevicePluginWifiDetector::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *p = static_cast<QProcess*>(sender());
    p->deleteLater();

    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        qWarning() << "error performing network scan:";
        qWarning() << p->readAllStandardError();
        return;
    }

    QList<Device*> watchedDevices = deviceManager()->findConfiguredDevices(supportedDevices().first().id());
    if (watchedDevices.isEmpty()) {
        return;
    }

    QStringList foundDevices;
    while(p->canReadLine()) {
        QString result = QString::fromLatin1(p->readLine());
        if (result.startsWith("MAC Address:")) {
            QStringList lineParts = result.split(' ');
            if (lineParts.count() > 3) {
                QString addr = lineParts.at(2);
                foundDevices << addr.toLower();
            }
        }
    }

    foreach (Device *device, watchedDevices) {
        bool wasInRange = device->stateValue(inRangeStateTypeId).toBool();
        bool wasFound = foundDevices.contains(device->paramValue("mac").toString().toLower());
        if (wasInRange != wasFound) {
            device->setStateValue(inRangeStateTypeId, wasFound);
        }
    }
}
