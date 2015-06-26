/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef DEVICEPLUGINDATETIME_H
#define DEVICEPLUGINDATETIME_H

#include "plugin/deviceplugin.h"
#include <QDateTime>
#include <QTimeZone>
#include <QTimer>

class DevicePluginDateTime : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugindatetime.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginDateTime();

    DeviceManager::HardwareResources requiredHardware() const override;
    QList<ParamType> configurationDescription() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void startMonitoringAutoDevices() override;


private:
    QTimer *m_timer;
    QTimeZone m_timeZone;

private slots:
    void timeout();
    void onConfigValueChanged(const QString &paramName, const QVariant &value);



};

#endif // DEVICEPLUGINDATETIME_H
