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
QUuid mockEvent2Id = QUuid("863d5920-b1cf-4eb9-88bd-8f7b8583b1cf");
QUuid mockIntStateId = QUuid("80baec19-54de-4948-ac46-31eabfaceb83");
QUuid mockBoolStateId = QUuid("9dd6a97c-dfd1-43dc-acbd-367932742310");

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

    QList<StateType> mockStates;

    StateType intState(mockIntStateId);
    intState.setName("intState");
    intState.setType(QVariant::Int);
    intState.setDefaultValue(10);
    mockStates.append(intState);

    StateType boolState(mockBoolStateId);
    boolState.setName("boolState");
    boolState.setType(QVariant::Int);
    boolState.setDefaultValue(false);
    mockStates.append(boolState);

    deviceClassMock.setStates(mockStates);

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

    EventType event2(mockEvent2Id);
    event2.setName("event2");
//    event2.setParameters(detectorEventParams);
    mockEvents.append(event2);

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

    connect(daemon, SIGNAL(triggerEvent(QUuid)), SLOT(triggerEvent(QUuid)));
    connect(daemon, SIGNAL(setState(QUuid, QVariant)), SLOT(setState(QUuid,QVariant)));

    return true;
}

void DevicePluginMock::setState(const QUuid &stateTypeId, const QVariant &value)
{
    qDebug() << "should set state" << stateTypeId << value;
}

void DevicePluginMock::triggerEvent(const QUuid &id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }

    Device *device = m_daemons.key(daemon);

    Event event(id, device->id(), QVariantMap());

    qDebug() << "Emitting event " << event.eventTypeId();
    emit emitEvent(event);
}
