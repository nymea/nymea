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

#ifndef DEVICEPLUGINMOCK_H
#define DEVICEPLUGINMOCK_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class HttpDaemon;

class DevicePluginMock : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.guh.DevicePlugin" FILE "devicepluginmock.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMock();

    QList<Vendor> supportedVendors() const override;
    QList<DeviceClass> supportedDevices() const override;
    DeviceManager::HardwareResources requiredHardware() const override;

    QString pluginName() const override;
    QUuid pluginId() const override;

    bool deviceCreated(Device *device) override;
    void deviceRemoved(Device *device) override;

    bool configureAutoDevice(QList<Device *> loadedDevices, Device *device) const override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private slots:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &id);

private:
    QHash<Device*, HttpDaemon*> m_daemons;
};

#endif // DEVICEPLUGINMOCK_H
