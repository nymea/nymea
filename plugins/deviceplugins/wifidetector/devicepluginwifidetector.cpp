/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "devicepluginwifidetector.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>

extern VendorId guhVendorId;
QUuid pluginUuid = QUuid("8e0f791e-b273-4267-8605-b7c2f55a68ab");
DeviceClassId detectorId = DeviceClassId("bd216356-f1ec-4324-9785-6982d2174e17");
StateTypeId inRangeStateTypeId = StateTypeId("cb43e1b5-4f61-4538-bfa2-c33055c542cf");

DevicePluginWifiDetector::DevicePluginWifiDetector()
{
}

QList<Vendor> DevicePluginWifiDetector::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor guh(guhVendorId, "guh");
    ret.append(guh);
    return ret;
}

QList<DeviceClass> DevicePluginWifiDetector::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassWifiDetector(pluginId(), guhVendorId, detectorId);
    deviceClassWifiDetector.setName("WiFi Device");
    
    QVariantList detectorParams;
    QVariantMap macParam;
    macParam.insert("name", "mac");
    macParam.insert("type", "string");
    detectorParams.append(macParam);

    deviceClassWifiDetector.setParams(detectorParams);

    QList<StateType> detectorStates;

    StateType inRangeState(inRangeStateTypeId);
    inRangeState.setName("inRange");
    inRangeState.setType(QVariant::Bool);
    inRangeState.setDefaultValue(false);
    detectorStates.append(inRangeState);

    deviceClassWifiDetector.setStates(detectorStates);

    ret.append(deviceClassWifiDetector);

    return ret;
}

DeviceManager::HardwareResources DevicePluginWifiDetector::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QString DevicePluginWifiDetector::pluginName() const
{
    return "WiFi Detector";
}

QUuid DevicePluginWifiDetector::pluginId() const
{
    return pluginUuid;
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
        bool wasFound = foundDevices.contains(device->params().value("mac").toString().toLower());
        if (wasInRange != wasFound) {
            device->setStateValue(inRangeStateTypeId, wasFound);
        }
    }
}
