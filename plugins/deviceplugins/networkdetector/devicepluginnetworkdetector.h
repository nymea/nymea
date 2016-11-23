/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2016 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEPLUGINNETWORKDETECTOR_H
#define DEVICEPLUGINNETWORKDETECTOR_H

#include "plugin/deviceplugin.h"
#include "host.h"

#include <QProcess>
#include <QXmlStreamReader>

class DevicePluginNetworkDetector : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginnetworkdetector.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginNetworkDetector();

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::HardwareResources requiredHardware() const override;

    void guhTimer() override;

private:
    QProcess * m_discoveryProcess;
    QProcess * m_scanProcess;

    QXmlStreamReader m_reader;

    QStringList getDefaultTargets();
    QProcess *startScanProcesses();

    // Process parsing
    QList<Host> parseProcessOutput(const QByteArray &processData);
    Host parseHost();

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // DEVICEPLUGINNETWORKDETECTOR_H
