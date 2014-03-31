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

#include "devicepluginmockdevice.h"

#include "device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>

QUuid pluginUuid = QUuid("2ce2ebc6-7dbb-4b89-ad67-6226aa955041");
QUuid mockWifiDetectorId = QUuid("37279e41-a478-43fa-92b4-c889db578670");
QUuid inRangeStateTypeId = QUuid("110deaf9-5615-4e08-942b-d5443a3bf965");
QUuid inRangeEventTypeId = QUuid("7f77120e-b3d1-493f-936e-9d86d7489785");

DevicePluginMockDevice::DevicePluginMockDevice()
{
}

QList<DeviceClass> DevicePluginMockDevice::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassMockWifiDetector(pluginId(), mockWifiDetectorId);
    deviceClassMockWifiDetector.setName("Mock WiFi Device");
    
    QVariantList detectorParams;
    QVariantMap macParam;
    macParam.insert("name", "mac");
    macParam.insert("type", "string");
    detectorParams.append(macParam);

    deviceClassMockWifiDetector.setParams(detectorParams);

    QList<StateType> detectorStates;

    StateType inRangeState(inRangeStateTypeId);
    inRangeState.setName("inRange");
    inRangeState.setType(QVariant::Bool);
    inRangeState.setDefaultValue(false);
    detectorStates.append(inRangeState);

    deviceClassMockWifiDetector.setStates(detectorStates);

    QList<EventType> detectorEvents;
    
    QVariantList detectorEventParams;
    QVariantMap paramInRange;
    paramInRange.insert("name", "inRange");
    paramInRange.insert("type", "bool");
    detectorEventParams.append(paramInRange);

    EventType inRangeEvent(inRangeEventTypeId);
    inRangeEvent.setName("inRange");
    inRangeEvent.setParameters(detectorEventParams);
    detectorEvents.append(inRangeEvent);

    deviceClassMockWifiDetector.setEvents(detectorEvents);
    ret.append(deviceClassMockWifiDetector);

    return ret;
}

DeviceManager::HardwareResources DevicePluginMockDevice::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QString DevicePluginMockDevice::pluginName() const
{
    return "WiFi Detector";
}

QUuid DevicePluginMockDevice::pluginId() const
{
    return pluginUuid;
}

void DevicePluginMockDevice::guhTimer()
{
}

