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

#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "devicemanager.h"
#include "deviceclass.h"

#include "typeutils.h"
#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"

#include <QObject>

class DeviceManager;
class Device;

class DevicePlugin: public QObject
{
    Q_OBJECT
public:
    DevicePlugin(QObject *parent = 0);
    virtual ~DevicePlugin();

    virtual void init() {}

    virtual QString pluginName() const = 0;
    virtual PluginId pluginId() const = 0;

    virtual QList<Vendor> supportedVendors() const = 0;
    virtual QList<DeviceClass> supportedDevices() const = 0;
    virtual DeviceManager::HardwareResources requiredHardware() const = 0;

    virtual bool configureAutoDevice(QList<Device *> loadedDevices, Device *device) const;
    virtual DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const QVariantMap &params) const;

    virtual bool deviceCreated(Device *device);
    virtual void deviceRemoved(Device *device);

    // Hardware input
    virtual void radioData(QList<int> rawData) {Q_UNUSED(rawData)}
    virtual void guhTimer() {}

    virtual QVariantMap configuration() const;
    virtual void setConfiguration(const QVariantMap &configuration);

public slots:
    virtual DeviceManager::DeviceError executeAction(Device *device, const Action &action) {
        Q_UNUSED(device) Q_UNUSED(action)
        return DeviceManager::DeviceErrorNoError;
    }

signals:
    void emitEvent(const Event &event);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);

protected:
    DeviceManager *deviceManager() const;
    QList<Device*> myDevices() const;
    Device* findDeviceByParams(const QVariantMap &params) const;

    void transmitData(QList<int> rawData);

private:
    void initPlugin(DeviceManager *deviceManager);

    DeviceManager *m_deviceManager;

    friend class DeviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "org.guh.DevicePlugin")

#endif
