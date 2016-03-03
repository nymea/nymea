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

/*!
    \page kodi.html
    \title Kodi - Media Center

    \ingroup plugins
    \ingroup network

    This plugin allowes you to controll the media center \l{http://kodi.tv/}{Kodi}. If you want to discover
    and control Kodi with guh, you need to activate the remote access and the UPnP service.

    \chapter "Activate UPnP"
    In order to discover Kodi in the network, you need to activate the UPnP serive in the Kodi settings:

    \section2 Settings
    \image kodi_settings.png

    \section2 Settings \unicode{0x2192} Services
    \image kodi_services.png

    \section2 Settings \unicode{0x2192} Services  \unicode{0x2192} UPnP
    Activate all options.
    \image kodi_upnp.png


    \chapter Activate "Remote Control"
    In order to control Kodi over the network with guh, you need to activate the remote control permissions:

    \section2 Settings \unicode{0x2192} Services  \unicode{0x2192} Remote Control
    Activate all options.
    \image kodi_remote.png

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": true}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/kodi/devicepluginkodi.json
*/

#include "devicepluginkodi.h"
#include "plugin/device.h"
#include "plugininfo.h"

DevicePluginKodi::DevicePluginKodi()
{
//    Q_INIT_RESOURCE(images);
//    QFile file(":/images/guh-logo.png");
//    if (!file.open(QIODevice::ReadOnly)) {
//        qCWarning(dcKodi) << "could not open" << file.fileName();
//        return;
//    }

//    QByteArray guhLogoByteArray = file.readAll();
//    if (guhLogoByteArray.isEmpty()) {
//        qCWarning(dcKodi) << "could not read" << file.fileName();
//        return;
//    }
//    m_logo = guhLogoByteArray;
}

DeviceManager::HardwareResources DevicePluginKodi::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceUpnpDisovery;
}

DeviceManager::DeviceSetupStatus DevicePluginKodi::setupDevice(Device *device)
{
    qCDebug(dcKodi) << "Setup Kodi device" << device->paramValue("ip").toString();
    Kodi *kodi= new Kodi(QHostAddress(device->paramValue("ip").toString()), 9090, this);

    connect(kodi, &Kodi::connectionStatusChanged, this, &DevicePluginKodi::onConnectionChanged);
    connect(kodi, &Kodi::stateChanged, this, &DevicePluginKodi::onStateChanged);
    connect(kodi, &Kodi::actionExecuted, this, &DevicePluginKodi::onActionExecuted);
    connect(kodi, &Kodi::versionDataReceived, this, &DevicePluginKodi::versionDataReceived);
    connect(kodi, &Kodi::updateDataReceived, this, &DevicePluginKodi::onSetupFinished);
    connect(kodi, &Kodi::onPlayerPlay, this, &DevicePluginKodi::onPlayerPlay);
    connect(kodi, &Kodi::onPlayerPause, this, &DevicePluginKodi::onPlayerPause);
    connect(kodi, &Kodi::onPlayerStop, this, &DevicePluginKodi::onPlayerStop);

    m_kodis.insert(kodi, device);
    m_asyncSetups.append(kodi);

    kodi->connectKodi();

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginKodi::deviceRemoved(Device *device)
{
    Kodi *kodi = m_kodis.key(device);
    m_kodis.remove(kodi);
    qCDebug(dcKodi) << "Delete " << device->name();
    kodi->deleteLater();
}

void DevicePluginKodi::guhTimer()
{
    foreach (Kodi *kodi, m_kodis.keys()) {
        if (!kodi->connected()) {
            kodi->connectKodi();
            continue;
        } else {
            // no need for polling information, notifications do the job
            //kodi->update();
        }
    }
}


DeviceManager::DeviceError DevicePluginKodi::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)
    Q_UNUSED(deviceClassId)
    qCDebug(dcKodi) << "Start UPnP search";
    upnpDiscover();
    return DeviceManager::DeviceErrorAsync;
}

void DevicePluginKodi::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const UpnpDeviceDescriptor &upnpDescriptor, upnpDeviceDescriptorList) {
        if (upnpDescriptor.modelName().contains("Kodi")) {

            // check if we allready found the kodi on this ip
            bool alreadyAdded = false;
            foreach (const DeviceDescriptor dDescriptor, deviceDescriptors) {
                if (dDescriptor.params().paramValue("ip").toString() == upnpDescriptor.hostAddress().toString()) {
                    alreadyAdded = true;
                    break;
                }
            }
            if (alreadyAdded)
                continue;

            qCDebug(dcKodi) << upnpDescriptor;
            DeviceDescriptor deviceDescriptor(kodiDeviceClassId, "Kodi - Media Center", upnpDescriptor.hostAddress().toString());
            ParamList params;
            params.append(Param("name", upnpDescriptor.friendlyName()));
            params.append(Param("ip", upnpDescriptor.hostAddress().toString()));
            params.append(Param("port", 9090));
            deviceDescriptor.setParams(params);
            deviceDescriptors.append(deviceDescriptor);
        }
    }
    emit devicesDiscovered(kodiDeviceClassId, deviceDescriptors);
}

DeviceManager::DeviceError DevicePluginKodi::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == kodiDeviceClassId) {
        Kodi *kodi = m_kodis.key(device);

        // check connection state
        if (!kodi->connected()) {
            return DeviceManager::DeviceErrorHardwareNotAvailable;
        }

        if (action.actionTypeId() == showNotificationActionTypeId) {
            kodi->showNotification(action.param("message").value().toString(), 8000, action.param("type").value().toString(), action.id());
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == volumeActionTypeId) {
            kodi->setVolume(action.param("volume").value().toInt(), action.id());
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == muteActionTypeId) {
            kodi->setMuted(action.param("mute").value().toBool(), action.id());
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == pressButtonActionTypeId) {
            kodi->pressButton(action.param("button").value().toString(), action.id());
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == systemActionTypeId) {
            kodi->systemCommand(action.param("command").value().toString(), action.id());
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == videoLibraryActionTypeId) {
            kodi->videoLibrary(action.param("command").value().toString(), action.id());
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == audioLibraryActionTypeId) {
            kodi->audioLibrary(action.param("command").value().toString(), action.id());
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginKodi::onConnectionChanged()
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);

    if (kodi->connected()) {
        // if this is the first setup, check version
        if (m_asyncSetups.contains(kodi)) {
            m_asyncSetups.removeAll(kodi);
            kodi->checkVersion();
        }
    }

    device->setStateValue(connectedStateTypeId, kodi->connected());
}

void DevicePluginKodi::onStateChanged()
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);

    // set device state values
    device->setStateValue(volumeStateTypeId, kodi->volume());
    device->setStateValue(muteStateTypeId, kodi->muted());
}

void DevicePluginKodi::onActionExecuted(const ActionId &actionId, const bool &success)
{
    if (success) {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
    } else {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorInvalidParameter);
    }
}

void DevicePluginKodi::versionDataReceived(const QVariantMap &data)
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);

    QVariantMap version = data.value("version").toMap();
    QString apiVersion = QString("%1.%2.%3").arg(version.value("major").toString()).arg(version.value("minor").toString()).arg(version.value("patch").toString());
    qCDebug(dcKodi) << "API Version:" << apiVersion;

    if (version.value("major").toInt() < 6) {
        qCWarning(dcKodi) << "incompatible api version:" << apiVersion;
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        return;
    }
    kodi->update();
}

void DevicePluginKodi::onSetupFinished(const QVariantMap &data)
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);

    QVariantMap version = data.value("version").toMap();
    QString kodiVersion = QString("%1.%2 (%3)").arg(version.value("major").toString()).arg(version.value("minor").toString()).arg(version.value("tag").toString());
    qCDebug(dcKodi) << "Version:" << kodiVersion;

    emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);

    kodi->showNotification("Connected", 2000, "info", ActionId());
}

void DevicePluginKodi::onPlayerPlay()
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);
    emit emitEvent(Event(onPlayerPlayEventTypeId, device->id()));
}

void DevicePluginKodi::onPlayerPause()
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);
    emit emitEvent(Event(onPlayerPauseEventTypeId, device->id()));
}

void DevicePluginKodi::onPlayerStop()
{
    Kodi *kodi = static_cast<Kodi *>(sender());
    Device *device = m_kodis.value(kodi);
    emit emitEvent(Event(onPlayerStopEventTypeId, device->id()));
}

