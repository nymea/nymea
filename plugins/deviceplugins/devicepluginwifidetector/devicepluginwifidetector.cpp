#include "devicepluginwifidetector.h"

#include "device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>

QUuid pluginUuid = QUuid("8e0f791e-b273-4267-8605-b7c2f55a68ab");
QUuid detectorId = QUuid("bd216356-f1ec-4324-9785-6982d2174e17");
QUuid inRangeStateTypeId = QUuid("cb43e1b5-4f61-4538-bfa2-c33055c542cf");
QUuid inRangeTriggerTypeId = QUuid("7cae711a-a0af-41b4-b3bf-38d3e23b41ba");

DevicePluginWifiDetector::DevicePluginWifiDetector()
{
}

QList<DeviceClass> DevicePluginWifiDetector::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassWifiDetector(pluginId(), detectorId);
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

    QList<TriggerType> detectorTriggers;
    
    QVariantList detectorTriggerParams;
    QVariantMap paramInRange;
    paramInRange.insert("name", "inRange");
    paramInRange.insert("type", "bool");
    detectorTriggerParams.append(paramInRange);

    TriggerType inRangeTrigger(inRangeTriggerTypeId);
    inRangeTrigger.setName("inRange");
    inRangeTrigger.setParameters(detectorTriggerParams);
    detectorTriggers.append(inRangeTrigger);

    deviceClassWifiDetector.setTriggers(detectorTriggers);
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

void DevicePluginWifiDetector::hiveTimer()
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

            QVariantMap params;
            params.insert("inRange", wasFound);
            Trigger trigger(inRangeTriggerTypeId, device->id(), params);

            qDebug() << "Device" << device->name() << QStringLiteral("is now ") + (wasFound ? "in" : "out of") + " range";
            emit emitTrigger(trigger);
        }
    }
}
