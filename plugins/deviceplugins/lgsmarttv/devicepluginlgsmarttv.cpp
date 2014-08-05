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

#include "devicepluginlgsmarttv.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>

VendorId lgVendorId = VendorId("a9af9673-78db-4226-a16b-f34b304f7041");
DeviceClassId lgSmartTvDeviceClassId = DeviceClassId("1d41b5a8-74ff-4a12-b365-c7bbe610848f");



DevicePluginLgSmartTv::DevicePluginLgSmartTv()
{
    m_discovery = new TvDiscovery(this);

    connect(m_discovery,SIGNAL(discoveryDone(QList<TvDevice*>)),this,SLOT(discoveryDone(QList<TvDevice*>)));
}

QList<Vendor> DevicePluginLgSmartTv::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor lgVendor(lgVendorId, "Lg");
    ret.append(lgVendor);
    return ret;
}

QList<DeviceClass> DevicePluginLgSmartTv::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassLgSmartTv(pluginId(), lgVendorId, lgSmartTvDeviceClassId);
    deviceClassLgSmartTv.setName("Lg Smart Tv");
    deviceClassLgSmartTv.setCreateMethod(DeviceClass::CreateMethodDiscovery);
    deviceClassLgSmartTv.setSetupMethod(DeviceClass::SetupMethodDisplayPin);

    QList<ParamType> paramTypes;
    paramTypes.append(ParamType("name", QVariant::String));
    paramTypes.append(ParamType("uuid", QVariant::String));
    paramTypes.append(ParamType("model", QVariant::String));
    paramTypes.append(ParamType("host address", QVariant::String));
    paramTypes.append(ParamType("location", QVariant::String));
    paramTypes.append(ParamType("manufacturer", QVariant::String));
    paramTypes.append(ParamType("key", QVariant::String));

    deviceClassLgSmartTv.setParamTypes(paramTypes);

    ret.append(deviceClassLgSmartTv);

    return ret;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginLgSmartTv::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    qDebug() << "should discover devices with params:" << params;

    if(deviceClassId != lgSmartTvDeviceClassId){
        return report(DeviceManager::DeviceErrorDeviceClassNotFound);
    }

    m_discovery->discover(2000);

    return report(DeviceManager::DeviceErrorAsync);
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginLgSmartTv::setupDevice(Device *device)
{
    return reportDeviceSetup();
}

DeviceManager::HardwareResources DevicePluginLgSmartTv::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginLgSmartTv::executeAction(Device *device, const Action &action)
{
    return report();
}

QString DevicePluginLgSmartTv::pluginName() const
{
    return "Lg Smart Tv";
}

PluginId DevicePluginLgSmartTv::pluginId() const
{
    return PluginId("4ef7a68b-9da0-4c62-b9ac-f478dc6f9f52");
}

void DevicePluginLgSmartTv::guhTimer()
{

}

void DevicePluginLgSmartTv::discoveryDone(QList<TvDevice*> tvList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (TvDevice *device, tvList) {
        DeviceDescriptor descriptor(lgSmartTvDeviceClassId, "Lg Smart Tv", device->modelName());
        ParamList params;
        params.append(Param("name", device->name()));
        params.append(Param("uuid", device->uuid()));
        params.append(Param("model", device->modelName()));
        params.append(Param("host address", device->hostAddress().toString()));
        params.append(Param("location", device->location().toString()));
        params.append(Param("manufacturer", device->manufacturer()));
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }
    emit devicesDiscovered(lgSmartTvDeviceClassId, deviceDescriptors);
}


