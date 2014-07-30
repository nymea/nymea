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

#include "deviceplugineq-3.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "types/param.h"

#include <QDebug>

VendorId maxVendorId = VendorId("2cac0645-855e-44fa-837e-1cab0ae4304c");

PluginId eq3PluginUuid = PluginId("f324c43c-9680-48d8-852a-93b2227139b9");

DeviceClassId cubeDeviceClassId = DeviceClassId("1e892268-8bd7-442c-a001-bd4e2e6b2949");
StateTypeId dateTimeStateTypeId = StateTypeId("78aed123-ca8e-4e11-a823-52043c4a4370");

DevicePluginEQ3::DevicePluginEQ3()
{
    m_cubeDiscovery = new MaxCubeDiscovery(this);

    connect(m_cubeDiscovery,SIGNAL(cubesDetected(QList<MaxCube*>)),this,SLOT(discoveryDone(QList<MaxCube*>)));
}

QList<Vendor> DevicePluginEQ3::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor max(maxVendorId, "Max!");
    ret.append(max);
    return ret;
}

QList<DeviceClass> DevicePluginEQ3::supportedDevices() const
{
    QList<DeviceClass> ret;

    // Cube
    DeviceClass cubeDeviceClass(pluginId(),maxVendorId,cubeDeviceClassId);
    cubeDeviceClass.setName("Max! Cube LAN Gateway");
    cubeDeviceClass.setCreateMethod(DeviceClass::CreateMethodDiscovery);
    cubeDeviceClass.setSetupMethod(DeviceClass::SetupMethodJustAdd);

    // Params
    QList<ParamType> params;
    ParamType hostParam("host address", QVariant::String);
    params.append(hostParam);

    ParamType portParam("port", QVariant::Int);
    params.append(portParam);

    ParamType serialNumberParam("serial number", QVariant::String);
    params.append(serialNumberParam);

    ParamType firmwareParam("firmware version", QVariant::Int);
    params.append(firmwareParam);

    cubeDeviceClass.setParamTypes(params);

    // States
    QList<StateType> states;
    StateType dateTimeState(dateTimeStateTypeId);
    dateTimeState.setName("cube time [unixtime]");
    dateTimeState.setType(QVariant::UInt);
    dateTimeState.setDefaultValue(0);
    states.append(dateTimeState);

    cubeDeviceClass.setStateTypes(states);

    ret.append(cubeDeviceClass);

    return ret;
}

DeviceManager::HardwareResources DevicePluginEQ3::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QString DevicePluginEQ3::pluginName() const
{
    return "eQ-3";
}

PluginId DevicePluginEQ3::pluginId() const
{
    return eq3PluginUuid;
}

QList<ParamType> DevicePluginEQ3::configurationDescription() const
{
    QList<ParamType> params;
    return params;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginEQ3::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if(deviceClassId == cubeDeviceClassId){
        m_cubeDiscovery->detectCubes();
        return report(DeviceManager::DeviceErrorAsync);
    }
    return report(DeviceManager::DeviceErrorDeviceClassNotFound);
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginEQ3::setupDevice(Device *device)
{
    qDebug() << "setupDevice" << device->params();

    foreach (MaxCube *cube, m_cubes.keys()) {
        if(cube->serialNumber() == device->paramValue("serial number").toString()){
            qDebug() << cube->serialNumber() << " allready exists...";
            return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure,QString("Cube allready in added"));
        }
    }

    MaxCube *cube = new MaxCube(this,device->paramValue("serial number").toString(),QHostAddress(device->paramValue("host address").toString()),device->paramValue("port").toInt());
    m_cubes.insert(cube,device);

    connect(cube,SIGNAL(cubeConnectionStatusChanged(bool)),this,SLOT(cubeConnectionStatusChanged(bool)));

    cube->connectToCube();

    return reportDeviceSetup(DeviceManager::DeviceSetupStatusAsync);
}

void DevicePluginEQ3::deviceRemoved(Device *device)
{
    if (!m_cubes.values().contains(device)) {
        return;
    }

    MaxCube *cube = m_cubes.key(device);
    cube->disconnectFromCube();
    qDebug() << "remove cube " << cube->serialNumber();
    m_cubes.remove(cube);
    cube->deleteLater();
}

void DevicePluginEQ3::guhTimer()
{
    foreach (MaxCube *cube, m_cubes.keys()) {
        cube->refresh();
    }
}

QPair<DeviceManager::DeviceError, QString> DevicePluginEQ3::executeAction(Device *device, const Action &action)
{

}

void DevicePluginEQ3::cubeConnectionStatusChanged(const bool &connected)
{
    if(connected){
        MaxCube *cube = static_cast<MaxCube*>(sender());
        Device *device;
        if (m_cubes.contains(cube)) {
            device = m_cubes.value(cube);
            device->setName("Max! Cube " + cube->serialNumber());
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess, QString());
        }
    }
}

void DevicePluginEQ3::discoveryDone(const QList<MaxCube *> &cubeList)
{
    QList<DeviceDescriptor> retList;
    foreach (MaxCube *cube, cubeList) {
        DeviceDescriptor descriptor(cubeDeviceClassId, "Max! Cube LAN Gateway",cube->serialNumber());
        ParamList params;
        Param hostParam("host address", cube->hostAddress().toString());
        params.append(hostParam);
        Param portParam("port", cube->port());
        params.append(portParam);
        Param firmwareParam("firmware version", cube->firmware());
        params.append(firmwareParam);
        Param serialNumberParam("serial number", cube->serialNumber());
        params.append(serialNumberParam);

        descriptor.setParams(params);
        retList.append(descriptor);
    }
    emit devicesDiscovered(cubeDeviceClassId,retList);
}
