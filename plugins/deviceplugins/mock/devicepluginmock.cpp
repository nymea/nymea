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

#include "devicepluginmock.h"
#include "httpdaemon.h"

#include "device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>

QUuid mockEvent1Id = QUuid("45bf3752-0fc6-46b9-89fd-ffd878b5b22b");

DevicePluginMock::DevicePluginMock()
{
}

QList<DeviceClass> DevicePluginMock::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassMock(pluginId(), QUuid("753f0d32-0468-4d08-82ed-1964aab03298"));
    deviceClassMock.setName("Mock Device");

    QVariantList mockParams;
    QVariantMap portParam;
    portParam.insert("name", "httpport");
    portParam.insert("type", "int");
    mockParams.append(portParam);

    deviceClassMock.setParams(mockParams);

//    QList<StateType> detectorStates;

//    StateType inRangeState(inRangeStateTypeId);
//    inRangeState.setName("inRange");
//    inRangeState.setType(QVariant::Bool);
//    inRangeState.setDefaultValue(false);
//    detectorStates.append(inRangeState);

//    deviceClassWifiDetector.setStates(detectorStates);

    QList<EventType> mockEvents;
    
//    QVariantList detectorEventParams;
//    QVariantMap paramInRange;
//    paramInRange.insert("name", "inRange");
//    paramInRange.insert("type", "bool");
//    detectorEventParams.append(paramInRange);

    EventType event1(mockEvent1Id);
    event1.setName("event1");
//    event1.setParameters(detectorEventParams);
    mockEvents.append(event1);

    deviceClassMock.setEvents(mockEvents);

    ret.append(deviceClassMock);

    return ret;
}

DeviceManager::HardwareResources DevicePluginMock::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QString DevicePluginMock::pluginName() const
{
    return "Mock Devices";
}

QUuid DevicePluginMock::pluginId() const
{
    return QUuid("727a4a9a-c187-446f-aadf-f1b2220607d1");
}

bool DevicePluginMock::deviceCreated(Device *device)
{
    qDebug() << "Mockdevice created returning true" << device->params().value("httpport").toInt();

    HttpDaemon *daemon = new HttpDaemon(device, this);
    m_daemons.insert(device, daemon);

    if (!daemon->isListening()) {
        qDebug() << "couldn't setup mockdevice";
        return false;
    }

    connect(daemon, SIGNAL(triggerEvent(int)), SLOT(triggerEvent(int)));

    return true;
}

void DevicePluginMock::triggerEvent(int id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }

    Device *device = m_daemons.key(daemon);

    Event event(mockEvent1Id, device->id(), QVariantMap());

    qDebug() << "Emitting event " << event.eventTypeId();
    emit emitEvent(event);
}
