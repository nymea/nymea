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

#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "devicemanager.h"
#include "deviceclass.h"

#include "typeutils.h"
#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"
#include "types/param.h"

#include <QObject>
#include <QJsonObject>

class DeviceManager;
class Device;

class DevicePlugin: public QObject
{
    Q_OBJECT
public:
    DevicePlugin(QObject *parent = 0);
    virtual ~DevicePlugin();

    virtual void init() {}

    QString pluginName() const;
    PluginId pluginId() const;
    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices() const;

    virtual DeviceManager::HardwareResources requiredHardware() const = 0;

    virtual void startMonitoringAutoDevices();
    virtual QPair<DeviceManager::DeviceError, QString> discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);

    virtual QPair<DeviceManager::DeviceSetupStatus, QString> setupDevice(Device *device);
    virtual void deviceRemoved(Device *device);

    virtual QPair<DeviceManager::DeviceSetupStatus, QString> confirmPairing(const QUuid &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params);

    // Hardware input
    virtual void radioData(const QList<int> &rawData) {Q_UNUSED(rawData)}
    virtual void guhTimer() {}

    // Configuration
    virtual QList<ParamType> configurationDescription() const;
    QPair<DeviceManager::DeviceError, QString> setConfiguration(const ParamList &configuration);
    ParamList configuration() const;
    QVariant configValue(const QString &paramName) const;
    QPair<DeviceManager::DeviceError, QString> setConfigValue(const QString &paramName, const QVariant &value);

public slots:
    virtual QPair<DeviceManager::DeviceError, QString> executeAction(Device *device, const Action &action) {
        Q_UNUSED(device) Q_UNUSED(action)
        return qMakePair<DeviceManager::DeviceError, QString>(DeviceManager::DeviceErrorNoError, "");
    }

signals:
    void emitEvent(const Event &event);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void deviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status, const QString &errorMessage);
    void pairingFinished(const QUuid &pairingTransactionId, DeviceManager::DeviceSetupStatus status, const QString &errorMessage);
    void actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status, const QString &errorMessage);
    void configValueChanged(const QString &paramName, const QVariant &value);
    void autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);

protected:
    DeviceManager *deviceManager() const;
    QList<Device*> myDevices() const;
    Device* findDeviceByParams(const ParamList &params) const;

    bool transmitData(int delay, QList<int> rawData);

    QPair<DeviceManager::DeviceError, QString> report(DeviceManager::DeviceError error = DeviceManager::DeviceErrorNoError, const QString &message = QString());
    QPair<DeviceManager::DeviceSetupStatus, QString> reportDeviceSetup(DeviceManager::DeviceSetupStatus status = DeviceManager::DeviceSetupStatusSuccess, const QString &message = QString());
private:
    void initPlugin(const QJsonObject &metaData, DeviceManager *deviceManager);

    DeviceManager *m_deviceManager;

    ParamList m_config;

    QJsonObject m_metaData;

    friend class DeviceManager;
};
Q_DECLARE_INTERFACE(DevicePlugin, "guru.guh.DevicePlugin")

#endif
