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


#include "deviceplugincommandlauncher.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>

DeviceClassId applicationDeviceClassId = DeviceClassId("0f39fe9e-51ea-4230-9646-2482c6234791");
DeviceClassId scriptDeviceClassId = DeviceClassId("96044325-a6fb-47c9-9117-f29c3b327978");
StateTypeId runningStateTypeId = StateTypeId("28d7e933-ff05-4f4c-95a0-482689543de5");
ActionTypeId executeActionTypeId = ActionTypeId("cf52b41d-3108-423c-8907-ca5b4d97cac5");
ActionTypeId killActionTypeId = ActionTypeId("d21b1fed-1dd9-4c5a-a64e-0c6ba94059be");

DevicePluginCommandLauncher::DevicePluginCommandLauncher()
{
}

DeviceManager::DeviceSetupStatus DevicePluginCommandLauncher::setupDevice(Device *device)
{
    // Application
    if(device->deviceClassId() == applicationDeviceClassId){
        device->setName("Application launcher (" + device->paramValue("name").toString() + ")");
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    // Script
    if(device->deviceClassId() == scriptDeviceClassId){
        QStringList scriptArguments = device->paramValue("script").toString().split(QRegExp("[ \r\n][ \r\n]*"));
        // check if script exists and if it is executable
        QFileInfo fileInfo(scriptArguments.first());
        if (!fileInfo.exists()) {
            qWarning() << "ERROR: script " << scriptArguments.first() << "does not exist.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
        if (!fileInfo.isExecutable()) {
            qWarning() << "ERROR: script " << scriptArguments.first() << "is not executable. Please check the permissions.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
        if (!fileInfo.isReadable()) {
            qWarning() << "ERROR: script " << scriptArguments.first() << "is not readable. Please check the permissions.";
            return DeviceManager::DeviceSetupStatusFailure;
        }

        device->setName("Bashscript launcher (" + device->paramValue("name").toString() + ")");
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::HardwareResources DevicePluginCommandLauncher::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginCommandLauncher::executeAction(Device *device, const Action &action)
{
    // Application
    if (device->deviceClassId() == applicationDeviceClassId ) {
        // execute application...
        if (action.actionTypeId() == executeActionTypeId) {
            // check if we allready have started the application
            if (m_applications.values().contains(device)) {
                if (m_applications.key(device)->state() == QProcess::Running) {
                    return DeviceManager::DeviceErrorDeviceInUse;
                }
            }
            QProcess *process = new QProcess(this);
            connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(applicationFinished(int,QProcess::ExitStatus)));
            connect(process, &QProcess::stateChanged, this, &DevicePluginCommandLauncher::applicationStateChanged);

            m_applications.insert(process, device);
            m_startingApplications.insert(process, action.id());
            process->start("/bin/bash", QStringList() << "-c" << device->paramValue("command").toString());

            return DeviceManager::DeviceErrorAsync;
        }
        // kill application...
        if (action.actionTypeId() == killActionTypeId) {
            // check if the application is running...
            if (!m_applications.values().contains(device)) {
                return DeviceManager::DeviceErrorNoError;
            }
            QProcess *process = m_applications.key(device);
            m_killingApplications.insert(process,action.id());
            process->kill();

            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // Script
    if (device->deviceClassId() == scriptDeviceClassId ) {
        // execute script...
        if (action.actionTypeId() == executeActionTypeId) {
            // check if we allready have started the script
            if (m_scripts.values().contains(device)) {
                if (m_scripts.key(device)->state() == QProcess::Running) {
                    return DeviceManager::DeviceErrorDeviceInUse;
                }
            }
            QProcess *process = new QProcess(this);
            connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(scriptFinished(int,QProcess::ExitStatus)));
            connect(process, &QProcess::stateChanged, this, &DevicePluginCommandLauncher::scriptStateChanged);

            m_scripts.insert(process, device);
            m_startingScripts.insert(process, action.id());
            process->start("/bin/bash", QStringList() << device->paramValue("script").toString());

            return DeviceManager::DeviceErrorAsync;
        }
        // kill script...
        if (action.actionTypeId() == killActionTypeId) {
            // check if the script is running...
            if (!m_scripts.values().contains(device)) {
                return DeviceManager::DeviceErrorNoError;
            }
            QProcess *process = m_scripts.key(device);
            m_killingScripts.insert(process,action.id());
            process->kill();

            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginCommandLauncher::deviceRemoved(Device *device)
{
    if (m_applications.values().contains(device)) {
        QProcess * process = m_applications.key(device);
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
        m_applications.remove(process);

        if (m_startingApplications.contains(process)) {
            m_startingApplications.remove(process);
        }
        if (m_killingApplications.contains(process)) {
            m_killingApplications.remove(process);
        }
        process->deleteLater();
    }
    if (m_scripts.values().contains(device)) {
        QProcess * process = m_scripts.key(device);
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
        m_scripts.remove(process);

        if (m_startingScripts.contains(process)) {
            m_startingScripts.remove(process);
        }
        if (m_killingScripts.contains(process)) {
            m_killingScripts.remove(process);
        }
        process->deleteLater();
    }
}

void DevicePluginCommandLauncher::scriptStateChanged(QProcess::ProcessState state)
{
    QProcess *process = static_cast<QProcess*>(sender());
    Device *device = m_scripts.value(process);

    switch (state) {
    case QProcess::Running:
        device->setStateValue(runningStateTypeId, true);
        emit actionExecutionFinished(m_startingScripts.value(process), DeviceManager::DeviceErrorNoError);
        m_startingScripts.remove(process);
        break;
    case QProcess::NotRunning:
        device->setStateValue(runningStateTypeId, false);
        if (m_killingScripts.contains(process)) {
            emit actionExecutionFinished(m_killingScripts.value(process), DeviceManager::DeviceErrorNoError);
            m_killingScripts.remove(process);
        }
        break;
    default:
        break;
    }
}

void DevicePluginCommandLauncher::scriptFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    QProcess *process = static_cast<QProcess*>(sender());
    Device *device = m_scripts.value(process);

    device->setStateValue(runningStateTypeId, false);

    m_scripts.remove(process);
    process->deleteLater();
}

void DevicePluginCommandLauncher::applicationStateChanged(QProcess::ProcessState state)
{
    QProcess *process = static_cast<QProcess*>(sender());
    Device *device = m_applications.value(process);

    switch (state) {
    case QProcess::Running:
        device->setStateValue(runningStateTypeId, true);
        emit actionExecutionFinished(m_startingApplications.value(process), DeviceManager::DeviceErrorNoError);
        m_startingApplications.remove(process);
        break;
    case QProcess::NotRunning:
        device->setStateValue(runningStateTypeId, false);
        if (m_killingApplications.contains(process)) {
            emit actionExecutionFinished(m_killingApplications.value(process), DeviceManager::DeviceErrorNoError);
            m_killingApplications.remove(process);
        }
        break;
    default:
        break;
    }
}

void DevicePluginCommandLauncher::applicationFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    QProcess *process = static_cast<QProcess*>(sender());
    Device *device = m_applications.value(process);

    device->setStateValue(runningStateTypeId, false);

    m_applications.remove(process);
    process->deleteLater();
}
