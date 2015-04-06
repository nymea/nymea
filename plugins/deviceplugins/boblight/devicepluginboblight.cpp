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

/*!
    \page boblight.html
    \title Boblight

    \ingroup plugins
    \ingroup network

    This plugin allows to communicate with a \l{https://code.google.com/p/boblight/}{boblight} server
    running on localhost:19333. If a boblight server is running ,the configured light devices from the server will
    appear automatically in guh.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": true}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/boblight/devicepluginboblight.json
*/

#include "devicepluginboblight.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include "bobclient.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>

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
