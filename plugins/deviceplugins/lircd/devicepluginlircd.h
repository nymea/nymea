/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEPLUGINLIRCD_H
#define DEVICEPLUGINLIRCD_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class LircClient;

class DevicePluginLircd: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginlircd.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginLircd();

    DeviceManager::HardwareResources requiredHardware() const override;

//    QVariantMap configuration() const override;
//    void setConfiguration(const QVariantMap &configuration) override;

private slots:
    void buttonPressed(const QString &remoteName, const QString &buttonName, int repeat);

private:
    LircClient *m_lircClient;
//    QVariantMap m_config;
};

#endif // DEVICEPLUGINBOLIRCD_H
