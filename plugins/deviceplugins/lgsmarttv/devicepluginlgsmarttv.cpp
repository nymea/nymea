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
    \page lgsmarttv.html
    \title LG Smart Tv

    \ingroup plugins
    \ingroup network

    This plugin allows to interact with \l{http://www.lg.com/us/experience-tvs/smart-tv}{LG Smart Tv's}
    with the \l{http://developer.lgappstv.com/TV_HELP/index.jsp?topic=%2Flge.tvsdk.references.book%2Fhtml%2FUDAP%2FUDAP%2FLG+UDAP+2+0+Protocol+Specifications.htm}{LG UDAP 2.0 Protocol Specifications}.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/lgsmarttv/devicepluginlgsmarttv.json
*/

#include "devicepluginlgsmarttv.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>

DevicePluginLgSmartTv::DevicePluginLgSmartTv()
{
}

DeviceManager::HardwareResources DevicePluginLgSmartTv::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer |
            DeviceManager::HardwareResourceUpnpDisovery |
            DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceError DevicePluginLgSmartTv::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    if(deviceClassId != lgSmartTvDeviceClassId){
        return DeviceManager::DeviceErrorDeviceClassNotFound;
    }
    qCDebug(dcLgSmartTv) << "Start discovering";
    upnpDiscover("udap:rootservice","UDAP/2.0");
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginLgSmartTv::setupDevice(Device *device)
{
    if (device->deviceClassId() != lgSmartTvDeviceClassId) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    TvDevice *tvDevice = new TvDevice(QHostAddress(device->paramValue("host address").toString()),
                                      device->paramValue("port").toInt(), this);
    tvDevice->setUuid(device->paramValue("uuid").toString());

    // if the key is missing, this setup call comes from a pairing procedure
    if (device->paramValue("key") == QString()) {
        // check if we know the key from the pairing procedure
        if (!m_tvKeys.contains(device->paramValue("uuid").toString())) {
            qCWarning(dcLgSmartTv) << "could not find any pairing key";
            return DeviceManager::DeviceSetupStatusFailure;
        }
        // use the key from the pairing procedure
        QString key = m_tvKeys.value(device->paramValue("uuid").toString());

        tvDevice->setKey(key);
        device->setParamValue("key", key);
    } else {
        // add the key for editing
        if (!m_tvKeys.contains(device->paramValue("uuid").toString())) {
            m_tvKeys.insert(tvDevice->uuid(), tvDevice->key());
        }
    }

    connect(tvDevice, &TvDevice::stateChanged, this, &DevicePluginLgSmartTv::stateChanged);

    m_tvList.insert(tvDevice, device);
    pairTvDevice(device, true);

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginLgSmartTv::deviceRemoved(Device *device)
{
    if (!m_tvList.values().contains(device)) {
        return;
    }

    TvDevice *tvDevice= m_tvList.key(device);
    qCDebug(dcLgSmartTv) << "Remove device" << device->name();
    unpairTvDevice(device);
    m_tvList.remove(tvDevice);
    delete tvDevice;
}

void DevicePluginLgSmartTv::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (UpnpDeviceDescriptor upnpDeviceDescriptor, upnpDeviceDescriptorList) {
        if (!upnpDeviceDescriptor.friendlyName().contains("LG") || !upnpDeviceDescriptor.deviceType().contains("TV"))
            continue;
        qCDebug(dcLgSmartTv) << upnpDeviceDescriptor;
        DeviceDescriptor descriptor(lgSmartTvDeviceClassId, "Lg Smart Tv", upnpDeviceDescriptor.modelName());
        ParamList params;
        params.append(Param("name", upnpDeviceDescriptor.friendlyName()));
        params.append(Param("uuid", upnpDeviceDescriptor.uuid()));
        params.append(Param("model", upnpDeviceDescriptor.modelName()));
        params.append(Param("host address", upnpDeviceDescriptor.hostAddress().toString()));
        params.append(Param("port", upnpDeviceDescriptor.port()));
        params.append(Param("key", QString()));
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }
    emit devicesDiscovered(lgSmartTvDeviceClassId, deviceDescriptors);
}

DeviceManager::DeviceError DevicePluginLgSmartTv::executeAction(Device *device, const Action &action)
{
    TvDevice * tvDevice = m_tvList.key(device);

    if (!tvDevice->reachable()) {
        qCWarning(dcLgSmartTv) << "Device not reachable";
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    if (action.actionTypeId() == commandVolumeUpActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::VolUp);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandVolumeDownActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::VolDown);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandMuteActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Mute);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandChannelUpActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::ChannelUp);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandChannelDownActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::ChannelDown);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandPowerOffActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Power);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandArrowUpActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Up);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandArrowDownActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Down);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandArrowLeftActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Left);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandArrowRightActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Right);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandOkActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Ok);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandBackActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Back);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandHomeActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Home);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandInputSourceActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::ExternalInput);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandExitActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Exit);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandInfoActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::Info);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandMyAppsActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::MyApps);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else if(action.actionTypeId() == commandProgramListActionTypeId) {
        QPair<QNetworkRequest, QByteArray> request = tvDevice->createPressButtonRequest(TvDevice::ProgramList);
        QNetworkReply *reply = networkManagerPost(request.first, request.second);
        m_asyncActions.insert(reply, action.id());
    } else {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceError DevicePluginLgSmartTv::displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor)
{
    Q_UNUSED(pairingTransactionId)

    QHostAddress host = QHostAddress(deviceDescriptor.params().paramValue("host address").toString());
    int port = deviceDescriptor.params().paramValue("port").toInt();
    QPair<QNetworkRequest, QByteArray> request = TvDevice::createDisplayKeyRequest(host, port);
    QNetworkReply *reply = networkManagerPost(request.first, request.second);

    m_showPinReply.append(reply);
    return DeviceManager::DeviceErrorNoError;
}

DeviceManager::DeviceSetupStatus DevicePluginLgSmartTv::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret)
{
    Q_UNUSED(deviceClassId)

    QHostAddress host = QHostAddress(params.paramValue("host address").toString());
    int port = params.paramValue("port").toInt();
    QPair<QNetworkRequest, QByteArray> request = TvDevice::createPairingRequest(host, port, secret);
    QNetworkReply *reply = networkManagerPost(request.first, request.second);

    m_setupPairingTv.insert(reply, pairingTransactionId);
    m_tvKeys.insert(params.paramValue("uuid").toString(), secret);

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginLgSmartTv::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_showPinReply.contains(reply)) {
        m_showPinReply.removeAll(reply);
        if (status != 200) {
            qCWarning(dcLgSmartTv) << "display pin on TV request error:" << status << reply->errorString();
        }
    } else if (m_setupPairingTv.keys().contains(reply)) {
        PairingTransactionId pairingTransactionId = m_setupPairingTv.take(reply);
        if(status != 200) {
            qCWarning(dcLgSmartTv) << "pair TV request error:" << status << reply->errorString();
            emit pairingFinished(pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        } else {
            // End pairing before calling setupDevice, which will always try to pair
            QPair<QNetworkRequest, QByteArray> request = TvDevice::createEndPairingRequest(reply->request().url());
            QNetworkReply *reply = networkManagerPost(request.first, request.second);
            m_setupEndPairingTv.insert(reply, pairingTransactionId);
        }
    } else if (m_setupEndPairingTv.keys().contains(reply)) {
        PairingTransactionId pairingTransactionId = m_setupEndPairingTv.take(reply);
        if(status != 200) {
            qCWarning(dcLgSmartTv) << "end pairing TV request error:" << status << reply->errorString();
            emit pairingFinished(pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        } else {
            emit pairingFinished(pairingTransactionId, DeviceManager::DeviceSetupStatusSuccess);
        }
    } else if (m_asyncSetup.keys().contains(reply)) {
        Device *device = m_asyncSetup.take(reply);
        TvDevice *tv = m_tvList.key(device);
        if(status != 200) {
            qCWarning(dcLgSmartTv) << "Pair TV request error:" << status << reply->errorString();
            tv->setPaired(false);
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        } else {
            qCDebug(dcLgSmartTv) << "Paired TV successfully.";
            tv->setPaired(true);
            refreshTv(device);
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
        }
    } else if (m_deleteTv.contains(reply)) {
        m_deleteTv.removeAll(reply);
        if(status != 200) {
            qCWarning(dcLgSmartTv) << "Rnd pairing TV (device deleted) request error:" << status << reply->errorString();
        } else {
            qCDebug(dcLgSmartTv) << "End pairing TV (device deleted) successfully.";
        }
    } else if (m_volumeInfoRequests.keys().contains(reply)) {
        Device *device = m_volumeInfoRequests.take(reply);
        TvDevice *tv = m_tvList.key(device);
        if(status != 200) {
            tv->setReachable(false);
            qCWarning(dcLgSmartTv) << "Volume information request error:" << status << reply->errorString();
        } else {
            tv->setReachable(true);
            tv->onVolumeInformationUpdate(reply->readAll());
        }
    } else if (m_channelInfoRequests.keys().contains(reply)) {
        Device *device = m_channelInfoRequests.take(reply);
        TvDevice *tv = m_tvList.key(device);
        if(status != 200) {
            tv->setReachable(false);
            qCWarning(dcLgSmartTv) << "Channel information request error:" << status << reply->errorString();
        } else {
            tv->setReachable(true);
            tv->onChannelInformationUpdate(reply->readAll());
        }
    } else if (m_asyncActions.keys().contains(reply)) {
        ActionId actionId = m_asyncActions.value(reply);
        if(status != 200) {
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareNotAvailable);
            qCWarning(dcLgSmartTv) << "Action request error:" << status << reply->errorString();
        } else {
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
        }
    }
    reply->deleteLater();
}

void DevicePluginLgSmartTv::guhTimer()
{
    foreach (Device *device, m_tvList.values()) {
        TvDevice *tv = m_tvList.key(device);
        if (tv->paired()) {
            refreshTv(device);
        } else {
            pairTvDevice(device);
        }
    }
}

void DevicePluginLgSmartTv::pairTvDevice(Device *device, const bool &setup)
{
    QHostAddress host = QHostAddress(device->paramValue("host address").toString());
    int port = device->paramValue("port").toInt();
    QString key = device->paramValue("key").toString();
    QPair<QNetworkRequest, QByteArray> request = TvDevice::createPairingRequest(host, port, key);
    QNetworkReply *reply = networkManagerPost(request.first, request.second);
    if (setup) {
        m_asyncSetup.insert(reply, device);
    } else {
        m_pairRequests.insert(reply, device);
    }
}

void DevicePluginLgSmartTv::unpairTvDevice(Device *device)
{
    QHostAddress host = QHostAddress(device->paramValue("host address").toString());
    int port = device->paramValue("port").toInt();
    QPair<QNetworkRequest, QByteArray> request = TvDevice::createEndPairingRequest(host, port);
    QNetworkReply *reply = networkManagerPost(request.first, request.second);

    m_deleteTv.append(reply);
}

void DevicePluginLgSmartTv::refreshTv(Device *device)
{
    TvDevice *tv = m_tvList.key(device);
    // check volume information
    QNetworkReply *volumeReply = networkManagerGet(tv->createVolumeInformationRequest());
    m_volumeInfoRequests.insert(volumeReply, device);

    // check channel information
    QNetworkReply *channelReply = networkManagerGet(tv->createChannelInformationRequest());
    m_channelInfoRequests.insert(channelReply, device);
}

void DevicePluginLgSmartTv::stateChanged()
{
    TvDevice *tvDevice = static_cast<TvDevice*>(sender());
    Device *device = m_tvList.value(tvDevice);

    device->setStateValue(reachableStateTypeId, tvDevice->reachable());
    device->setStateValue(tv3DModeStateTypeId, tvDevice->is3DMode());
    device->setStateValue(tvVolumeLevelStateTypeId, tvDevice->volumeLevel());
    device->setStateValue(tvMuteStateTypeId, tvDevice->mute());
    device->setStateValue(tvChannelTypeStateTypeId, tvDevice->channelType());
    device->setStateValue(tvChannelNameStateTypeId, tvDevice->channelName());
    device->setStateValue(tvChannelNumberStateTypeId, tvDevice->channelNumber());
    device->setStateValue(tvProgramNameStateTypeId, tvDevice->programName());
    device->setStateValue(tvInputSourceIndexStateTypeId, tvDevice->inputSourceIndex());
    device->setStateValue(tvInputSourceLabelNameStateTypeId, tvDevice->inputSourceLabelName());
}
