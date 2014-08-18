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

#include "devicepluginboblight.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include "bobclient.h"

#include <QDebug>
#include <QStringList>

DeviceClassId boblightDeviceClassId = DeviceClassId("1647c61c-db14-461e-8060-8a3533d5d92f");
StateTypeId colorStateTypeId = StateTypeId("97ec80cd-43a9-40fa-93b7-d1580043d981");
ActionTypeId setColorActionTypeId = ActionTypeId("668e1aa3-fa13-49ce-8630-17a5c0a7c34b");

DevicePluginBoblight::DevicePluginBoblight()
{
    m_bobClient = new BobClient(this);
    connect(this, &DevicePlugin::configValueChanged, this, &DevicePluginBoblight::connectToBoblight);
}

DeviceManager::HardwareResources DevicePluginBoblight::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginBoblight::startMonitoringAutoDevices()
{
    if (!m_bobClient->connected()) {
        return;
    }

    QList<Device*> loadedDevices = deviceManager()->findConfiguredDevices(boblightDeviceClassId);

    QList<DeviceDescriptor> deviceDescriptorList;
    for (int i = loadedDevices.count(); i < m_bobClient->lightsCount(); i++) {
        DeviceDescriptor deviceDescriptor(boblightDeviceClassId, "Boblight Channel " + QString::number(i));
        ParamList params;
        Param param("channel");
        param.setValue(i);
        params.append(param);
        deviceDescriptor.setParams(params);
        deviceDescriptorList.append(deviceDescriptor);
    }
    emit autoDevicesAppeared(boblightDeviceClassId, deviceDescriptorList);
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginBoblight::setupDevice(Device *device)
{
    if (!m_bobClient->connected()) {
        return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure, "Cannot connect to Boblight");
    }

    m_bobClient->currentColor(device->paramValue("channel").toInt());
    return reportDeviceSetup();
}

QString DevicePluginBoblight::pluginName() const
{
    return "Boblight client";
}

PluginId DevicePluginBoblight::pluginId() const
{
    return boblightPluginUuid;
}

QList<ParamType> DevicePluginBoblight::configurationDescription() const
{
    QList<ParamType> params;
    params.append(ParamType("boblighthost", QVariant::String, "localhost"));
    params.append(ParamType("boblightport", QVariant::String, "19333"));
    return params;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginBoblight::executeAction(Device *device, const Action &action)
{
    if (!m_bobClient->connected()) {
        return report(DeviceManager::DeviceErrorSetupFailed, device->id().toString());
    }
    QColor newColor = action.param("color").value().value<QColor>();
    if (!newColor.isValid()) {
        return report(DeviceManager::DeviceErrorActionParameterError, "color");
    }
    qDebug() << "executing boblight action" << newColor;
    m_bobClient->setColor(device->paramValue("channel").toInt(), newColor);
    m_bobClient->sync();

    device->setStateValue(colorStateTypeId, newColor);
    return report();
}

void DevicePluginBoblight::connectToBoblight()
{
    if (configValue("boblighthost").isValid() && configValue("boblightport").isValid()) {
        m_bobClient->connect(configValue("boblighthost").toString(), configValue("boblightport").toInt());
    }
}
