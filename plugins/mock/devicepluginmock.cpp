/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \page mockdevices.html
    \title Mock devices
    \brief Devices for the nymea test base.

    \ingroup plugins
    \ingroup nymea-tests

    The mock devices are used for testing.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.
*/

#include "devicepluginmock.h"
#include "httpdaemon.h"

#include "devices/device.h"
#include "plugininfo.h"

#include "network/networkaccessmanager.h"

#include <QDebug>
#include <QColor>
#include <QStringList>
#include <QTimer>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QJsonDocument>

DevicePluginMock::DevicePluginMock()
{
    generateBrowseItems();
}

DevicePluginMock::~DevicePluginMock()
{
}

DeviceDiscoveryInfo DevicePluginMock::discoverDevices(DeviceDiscoveryInfo deviceDiscoveryInfo, const ParamList &params)
{
    if (deviceDiscoveryInfo.deviceClassId() == mockDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, [this, deviceDiscoveryInfo](){
            emitDevicesDiscovered(deviceDiscoveryInfo);
        });
        deviceDiscoveryInfo.setStatus(Device::DeviceErrorAsync);
        return deviceDiscoveryInfo;
    } else if (deviceDiscoveryInfo.deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock push button discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockPushButtonDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, [this, deviceDiscoveryInfo](){
            emitPushButtonDevicesDiscovered(deviceDiscoveryInfo);
        });
        deviceDiscoveryInfo.setStatus(Device::DeviceErrorAsync);
        return deviceDiscoveryInfo;
    } else if (deviceDiscoveryInfo.deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock display pin discovery:" << params;
        m_discoveredDeviceCount = params.paramValue(mockDisplayPinDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, this, [this, deviceDiscoveryInfo](){
            emitDisplayPinDevicesDiscovered(deviceDiscoveryInfo);
        });
        deviceDiscoveryInfo.setStatus(Device::DeviceErrorAsync);
        return deviceDiscoveryInfo;
    } else if (deviceDiscoveryInfo.deviceClassId() == mockParentDeviceClassId) {
        qCDebug(dcMockDevice()) << "Starting discovery for mock device parent";
        QTimer::singleShot(1000, this, [this, deviceDiscoveryInfo](){
            DeviceDescriptor descriptor(mockParentDeviceClassId, "Mock Parent (Discovered)");
            DeviceDiscoveryInfo ret(deviceDiscoveryInfo);
            ret.setDeviceDescriptors(DeviceDescriptors() << descriptor);
            ret.setStatus(Device::DeviceErrorNoError);
            emit devicesDiscovered(ret);
        });
        deviceDiscoveryInfo.setStatus(Device::DeviceErrorAsync);
        return deviceDiscoveryInfo;
    } else if (deviceDiscoveryInfo.deviceClassId() == mockChildDeviceClassId) {
        QTimer::singleShot(1000, this, [this, deviceDiscoveryInfo](){
            QList<DeviceDescriptor> descriptors;
            if (!myDevices().filterByDeviceClassId(mockParentDeviceClassId).isEmpty()) {
                Device *parent = myDevices().filterByDeviceClassId(mockParentDeviceClassId).first();
                DeviceDescriptor descriptor(mockChildDeviceClassId, "Mock Child (Discovered)", QString(), parent->id());
                descriptors.append(descriptor);
            }
            DeviceDiscoveryInfo ret(deviceDiscoveryInfo);
            ret.setDeviceDescriptors(descriptors);
            ret.setStatus(Device::DeviceErrorNoError);
            emit devicesDiscovered(ret);
        });
        deviceDiscoveryInfo.setStatus(Device::DeviceErrorAsync);
        return deviceDiscoveryInfo;
    }

    qCWarning(dcMockDevice()) << "Cannot discover for deviceClassId" << deviceDiscoveryInfo.deviceClassId();
    deviceDiscoveryInfo.setStatus(Device::DeviceErrorDeviceClassNotFound);
    return deviceDiscoveryInfo;
}

DeviceSetupInfo DevicePluginMock::setupDevice(Device *device)
{
    if (device->deviceClassId() == mockDeviceClassId || device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        bool async = false;
        bool broken = false;
        if (device->deviceClassId() == mockDeviceClassId) {
            async = device->paramValue(mockDeviceAsyncParamTypeId).toBool();
            broken = device->paramValue(mockDeviceBrokenParamTypeId).toBool();
        } else {
            async = device->paramValue(mockDeviceAutoDeviceAsyncParamTypeId).toBool();
            broken = device->paramValue(mockDeviceAutoDeviceBrokenParamTypeId).toBool();
        }

        if (broken) {
            qCWarning(dcMockDevice) << "This device is intentionally broken.";
            return DeviceSetupInfo(device->id(), Device::DeviceErrorHardwareFailure, QT_TR_NOOP("This mock device is intentionally broken."));
        }

        HttpDaemon *daemon = new HttpDaemon(device, this);
        m_daemons.insert(device, daemon);

        if (!daemon->isListening()) {
            qCWarning(dcMockDevice) << "HTTP port opening failed:" << device->paramValue(mockDeviceHttpportParamTypeId).toInt();
            return DeviceSetupInfo(device->id(), Device::DeviceErrorHardwareNotAvailable, QT_TR_NOOP("Failed to bind HTTP port. Port in use?"));
        }

        connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
        connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);
        // Keep this queued or it might happen that the HttpDaemon is deleted before it is able to reply to the caller
        connect(daemon, &HttpDaemon::disappear, this, &DevicePluginMock::onDisappear, Qt::QueuedConnection);
        connect(daemon, &HttpDaemon::reconfigureAutodevice, this, &DevicePluginMock::onReconfigureAutoDevice, Qt::QueuedConnection);

        if (async) {
            m_asyncSetupDevices.append(device);
            QTimer::singleShot(1000, this, SLOT(emitDeviceSetupFinished()));
            return DeviceSetupInfo(device->id(), Device::DeviceErrorAsync);
        }
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup PushButton mock device" << device->params();
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup DisplayPin mock device" << device->params();
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockParentDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Parent mock device" << device->params();
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockChildDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Child mock device" << device->params();
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockInputTypeDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup InputType mock device" << device->params();
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockOAuthGoogleDeviceClassId) {
        qCDebug(dcMockDevice()) << "Google OAuth setup complete";
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    } else if (device->deviceClassId() == mockOAuthSonosDeviceClassId) {
        qCDebug(dcMockDevice()) << "Sonos OAuth setup complete";
        return DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good."));
    }

    return DeviceSetupInfo(device->id(), Device::DeviceErrorDeviceClassNotFound);
}

void DevicePluginMock::postSetupDevice(Device *device)
{
    qCDebug(dcMockDevice) << "Postsetup mockdevice" << device->name();
    if (device->deviceClassId() == mockParentDeviceClassId) {
        foreach (Device *d, myDevices()) {
            if (d->deviceClassId() == mockChildDeviceClassId && d->parentId() == device->id()) {
                return;
            }
        }
        onChildDeviceDiscovered(device->id());
    }
}

void DevicePluginMock::deviceRemoved(Device *device)
{
    delete m_daemons.take(device);
}

void DevicePluginMock::startMonitoringAutoDevices()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == mockDeviceAutoDeviceClassId) {
            return; // We already have a Auto Mock device... do nothing.
        }
    }

    DeviceDescriptor mockDescriptor(mockDeviceAutoDeviceClassId, "Mock Device (Auto created)");

    ParamList params;
    qsrand(QDateTime::currentMSecsSinceEpoch());
    int port = 4242 + (qrand() % 1000);
    Param param(mockDeviceAutoDeviceHttpportParamTypeId, port);
    params.append(param);
    mockDescriptor.setParams(params);

    QList<DeviceDescriptor> deviceDescriptorList;
    deviceDescriptorList.append(mockDescriptor);

    emit autoDevicesAppeared(deviceDescriptorList);
}

DevicePairingInfo DevicePluginMock::pairDevice(DevicePairingInfo &devicePairingInfo)
{
    if (devicePairingInfo.deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << QString(tr("Pushbutton pairing request"));
        devicePairingInfo.setMessage(tr("Wait 3 second before you continue, the push button will be pressed automatically."));
        devicePairingInfo.setStatus(Device::DeviceErrorNoError);
        return devicePairingInfo;
    }
    if (devicePairingInfo.deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << QString(tr("Display pin!! The pin is 243681"));
        devicePairingInfo.setStatus(Device::DeviceErrorNoError);
        devicePairingInfo.setMessage(tr("Please enter the secret which normaly will be displayed on the device. For the mockdevice the pin is 243681."));
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockOAuthGoogleDeviceClassId) {
        QString clientId= "937667874529-pr6s5ciu6sfnnqmt2sppvb6rokbkjjta.apps.googleusercontent.com";
        QString clientSecret = "1ByBRmNqaK08VC54eEVcnGf1";

//        QString code_verifier = QUuid::createUuid().toString().remove(QRegExp("[{}]")) + QUuid::createUuid().toString().remove(QRegExp("[{}]"));
//        qCDebug(dcMockDevice()) << "Code verifier:" << code_verifier;

//        // plain
//        QString code_challenge_plain = code_verifier;
//        QString code_challenge_s256 = QByteArray(QCryptographicHash::hash(code_verifier.toLatin1(), QCryptographicHash::Sha256)).toBase64().toPercentEncoding();

        QUrl url("https://accounts.google.com/o/oauth2/v2/auth");
        QUrlQuery queryParams;
        queryParams.addQueryItem("client_id", clientId);
        queryParams.addQueryItem("redirect_uri", "https://127.0.0.1:8888");
        queryParams.addQueryItem("response_type", "code");
        queryParams.addQueryItem("scope", "profile email");
        queryParams.addQueryItem("state", "ya-ya");
        url.setQuery(queryParams);

        devicePairingInfo.setOAuthUrl(url);
        devicePairingInfo.setStatus(Device::DeviceErrorNoError);
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockOAuthSonosDeviceClassId) {
        QString clientId = "b15cbf8c-a39c-47aa-bd93-635a96e9696c";
        QString clientSecret = "c086ba71-e562-430b-a52f-867c6482fd11";

        QUrl url("https://api.sonos.com/login/v3/oauth");
        QUrlQuery queryParams;
        queryParams.addQueryItem("client_id", clientId);
        queryParams.addQueryItem("redirect_uri", "https://127.0.0.1:8888");
        queryParams.addQueryItem("response_type", "code");
        queryParams.addQueryItem("scope", "playback-control-all");
        queryParams.addQueryItem("state", "ya-ya");
        url.setQuery(queryParams);

        qCDebug(dcMockDevice()) << "Sonos url:" << url;

        devicePairingInfo.setOAuthUrl(url);
        devicePairingInfo.setStatus(Device::DeviceErrorNoError);
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockUserAndPassDeviceClassId) {
        devicePairingInfo.setMessage(tr("Please enter login credentials"));
        devicePairingInfo.setStatus(Device::DeviceErrorNoError);
        return devicePairingInfo;
    }

    qCWarning(dcMockDevice()) << "Unhandled pairing metod!";
    devicePairingInfo.setStatus(Device::DeviceErrorCreationMethodNotSupported);
    return devicePairingInfo;
}

DevicePairingInfo DevicePluginMock::confirmPairing(DevicePairingInfo &devicePairingInfo, const QString &username, const QString &secret)
{
    qCDebug(dcMockDevice) << "Confirm pairing";

    if (devicePairingInfo.deviceClassId() == mockPushButtonDeviceClassId) {
        if (!m_pushbuttonPressed) {
            qCDebug(dcMockDevice) << "PushButton not pressed yet!";
            devicePairingInfo.setMessage(tr("The push button does not have been pressed."));
            devicePairingInfo.setStatus(Device::DeviceErrorAuthenticationFailure);
            return devicePairingInfo;
        }

        m_pairingInfo = devicePairingInfo;
        QTimer::singleShot(1000, this, SLOT(onPushButtonPairingFinished()));
        devicePairingInfo.setStatus(Device::DeviceErrorAsync);
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockDisplayPinDeviceClassId) {
        if (secret != "243681") {
            qCWarning(dcMockDevice) << "Invalid pin:" << secret;
            devicePairingInfo.setMessage(tr("Invalid PIN"));
            devicePairingInfo.setStatus(Device::DeviceErrorAuthenticationFailure);
            return devicePairingInfo;
        }
        m_pairingInfo = devicePairingInfo;
        QTimer::singleShot(500, this, SLOT(onDisplayPinPairingFinished()));
        devicePairingInfo.setStatus(Device::DeviceErrorAsync);
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockOAuthGoogleDeviceClassId) {
        qCDebug(dcMockDevice()) << "Secret is" << secret;
        QUrl url(secret);
        QUrlQuery query(url);
        qCDebug(dcMockDevice()) << "Acess code is:" << query.queryItemValue("code");

        QString accessCode = query.queryItemValue("code");

        // Obtaining access token
        QString clientId= "937667874529-pr6s5ciu6sfnnqmt2sppvb6rokbkjjta.apps.googleusercontent.com";
        QString clientSecret = "1ByBRmNqaK08VC54eEVcnGf1";

        url = QUrl("https://www.googleapis.com/oauth2/v4/token");
        query.clear();
        query.addQueryItem("code", accessCode);
        query.addQueryItem("client_id", clientId);
        query.addQueryItem("client_secret", clientSecret);
        query.addQueryItem("grant_type", "authorization_code");
        query.addQueryItem("redirect_uri", "https%3A%2F%2F127.0.0.1%3A8888");
//        query.addQueryItem("code_verifier", codeVerifier);
        url.setQuery(query);

        QNetworkRequest request(url);

        QNetworkReply *reply = hardwareManager()->networkManager()->post(request, QByteArray());
        connect(reply, &QNetworkReply::finished, this, [this, reply, devicePairingInfo](){
            reply->deleteLater();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            qCDebug(dcMockDevice()) << "Sonos accessToken reply:" << this << reply->error() << reply->errorString() << jsonDoc.toJson();
            qCDebug(dcMockDevice()) << "Access token:" << jsonDoc.toVariant().toMap().value("access_token").toString();
            qCDebug(dcMockDevice()) << "expires at" << QDateTime::currentDateTime().addSecs(jsonDoc.toVariant().toMap().value("expires_in").toInt()).toString();
            qCDebug(dcMockDevice()) << "Refresh token:" << jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcMockDevice()) << "ID token:" << jsonDoc.toVariant().toMap().value("id_token").toString();
            DevicePairingInfo info(devicePairingInfo);
            info.setStatus(Device::DeviceErrorNoError);
            emit pairingFinished(info);
        });


        devicePairingInfo.setStatus(Device::DeviceErrorAsync);
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockOAuthSonosDeviceClassId) {
        qCDebug(dcMockDevice()) << "Secret is" << secret;
        QUrl url(secret);
        QUrlQuery query(url);
        qCDebug(dcMockDevice()) << "Acess code is:" << query.queryItemValue("code");

        QString accessCode = query.queryItemValue("code");

        // Obtaining access token
        url = QUrl("https://api.sonos.com/login/v3/oauth/access");
        query.clear();
        query.addQueryItem("grant_type", "authorization_code");
        query.addQueryItem("code", accessCode);
        query.addQueryItem("redirect_uri", "https%3A%2F%2F127.0.0.1%3A8888");
        url.setQuery(query);

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded;charset=utf-8");

        QByteArray clientId = "b15cbf8c-a39c-47aa-bd93-635a96e9696c";
        QByteArray clientSecret = "c086ba71-e562-430b-a52f-867c6482fd11";

        QByteArray auth = QByteArray(clientId + ':' + clientSecret).toBase64(QByteArray::Base64Encoding | QByteArray::KeepTrailingEquals);
        request.setRawHeader("Authorization", QString("Basic %1").arg(QString(auth)).toUtf8());

        QNetworkReply *reply = hardwareManager()->networkManager()->post(request, QByteArray());
        connect(reply, &QNetworkReply::finished, this, [this, reply, devicePairingInfo](){
            reply->deleteLater();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            qCDebug(dcMockDevice()) << "Sonos accessToken reply:" << this << reply->error() << reply->errorString() << jsonDoc.toJson();
            qCDebug(dcMockDevice()) << "Access token:" << jsonDoc.toVariant().toMap().value("access_token").toString();
            qCDebug(dcMockDevice()) << "expires at" << QDateTime::currentDateTime().addSecs(jsonDoc.toVariant().toMap().value("expires_in").toInt()).toString();
            qCDebug(dcMockDevice()) << "Refresh token:" << jsonDoc.toVariant().toMap().value("refresh_token").toString();
            DevicePairingInfo info(devicePairingInfo);
            info.setStatus(Device::DeviceErrorNoError);
            emit pairingFinished(info);
        });

        devicePairingInfo.setStatus(Device::DeviceErrorAsync);
        return devicePairingInfo;
    }

    if (devicePairingInfo.deviceClassId() == mockUserAndPassDeviceClassId) {
        qCDebug(dcMockDevice()) << "Credentials received:" << username << secret;
        if (username == "user" && secret == "password") {
            devicePairingInfo.setStatus(Device::DeviceErrorNoError);
            return devicePairingInfo;
        } else {
            devicePairingInfo.setMessage(tr("Wrong username or password"));
            devicePairingInfo.setStatus(Device::DeviceErrorAuthenticationFailure);
            return devicePairingInfo;
        }
    }

    qCWarning(dcMockDevice) << "Invalid deviceclassId -> no pairing possible with this device";
    devicePairingInfo.setStatus(Device::DeviceErrorHardwareFailure);
    return devicePairingInfo;
}

Device::BrowseResult DevicePluginMock::browseDevice(Device *device, Device::BrowseResult result, const QString &itemId, const QLocale &locale)
{
    Q_UNUSED(locale)
    qCDebug(dcMockDevice()) << "Browse device called" << device;
    if (device->deviceClassId() == mockDeviceClassId) {
        if (device->paramValue(mockDeviceAsyncParamTypeId).toBool()) {
            result.status = Device::DeviceErrorAsync;
            QTimer::singleShot(1000, device, [this, device, result, itemId]() mutable {
                if (device->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
                    result.status = Device::DeviceErrorHardwareFailure;
                } else {
                    VirtualFsNode *node = m_virtualFs->findNode(itemId);
                    if (!node) {
                        result.status = Device::DeviceErrorItemNotFound;
                        emit browseRequestFinished(result);
                        return;
                    }
                    foreach (VirtualFsNode *child, node->childs) {
                        result.items.append(child->item);
                    }
                    result.status = Device::DeviceErrorNoError;
                }
                emit browseRequestFinished(result);
            });
        }
        else if (device->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
            result.status = Device::DeviceErrorHardwareFailure;
        } else {
            VirtualFsNode *node = m_virtualFs->findNode(itemId);
            if (!node) {
                result.status = Device::DeviceErrorItemNotFound;
                return result;
            }
            foreach (VirtualFsNode *child, node->childs) {
                result.items.append(child->item);
            }
            result.status = Device::DeviceErrorNoError;
        }
    }
    return result;
}

Device::BrowserItemResult DevicePluginMock::browserItem(Device *device, Device::BrowserItemResult result, const QString &itemId, const QLocale &locale)
{
    Q_UNUSED(device)
    Q_UNUSED(locale)
    VirtualFsNode *node = m_virtualFs->findNode(itemId);
    if (!node) {
        result.status = Device::DeviceErrorItemNotFound;
        return result;
    }
    result.item = node->item;
    result.status = Device::DeviceErrorNoError;
    return result;
}

Device::DeviceError DevicePluginMock::executeAction(Device *device, const Action &action)
{
    if (!myDevices().contains(device))
        return Device::DeviceErrorDeviceNotFound;

    if (device->deviceClassId() == mockDeviceClassId) {
        if (action.actionTypeId() == mockAsyncActionTypeId || action.actionTypeId() == mockAsyncFailingActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return Device::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockFailingActionTypeId)
            return Device::DeviceErrorSetupFailed;

        if (action.actionTypeId() == mockPowerActionTypeId) {
            qCDebug(dcMockDevice()) << "Setting power to" << action.param(mockPowerActionPowerParamTypeId).value().toBool();
            device->setStateValue(mockPowerStateTypeId, action.param(mockPowerActionPowerParamTypeId).value().toBool());
        }
        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return Device::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockDeviceAutoDeviceClassId) {
        if (action.actionTypeId() == mockDeviceAutoMockActionAsyncActionTypeId || action.actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
            m_asyncActions.append(qMakePair<Action, Device*>(action, device));
            QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
            return Device::DeviceErrorAsync;
        }

        if (action.actionTypeId() == mockDeviceAutoMockActionBrokenActionTypeId)
            return Device::DeviceErrorSetupFailed;

        m_daemons.value(device)->actionExecuted(action.actionTypeId());
        return Device::DeviceErrorNoError;
    } else if (device->deviceClassId() == mockPushButtonDeviceClassId) {
        if (action.actionTypeId() == mockPushButtonColorActionTypeId) {
            QString colorString = action.param(mockPushButtonColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return Device::DeviceErrorInvalidParameter;
            }
            device->setStateValue(mockPushButtonColorStateTypeId, colorString);
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonPercentageActionTypeId) {
            device->setStateValue(mockPushButtonPercentageStateTypeId, action.param(mockPushButtonPercentageActionPercentageParamTypeId).value().toInt());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonAllowedValuesActionTypeId) {
            device->setStateValue(mockPushButtonAllowedValuesStateTypeId, action.param(mockPushButtonAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonDoubleActionTypeId) {
            device->setStateValue(mockPushButtonDoubleStateTypeId, action.param(mockPushButtonDoubleActionDoubleParamTypeId).value().toDouble());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonBoolActionTypeId) {
            device->setStateValue(mockPushButtonBoolStateTypeId, action.param(mockPushButtonBoolActionBoolParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockPushButtonTimeoutActionTypeId) {
            return Device::DeviceErrorAsync;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockDisplayPinDeviceClassId) {
        if (action.actionTypeId() == mockDisplayPinColorActionTypeId) {
            QString colorString = action.param(mockDisplayPinColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                return Device::DeviceErrorInvalidParameter;
            }
            device->setStateValue(mockDisplayPinColorStateTypeId, colorString);
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinPercentageActionTypeId) {
            device->setStateValue(mockDisplayPinPercentageStateTypeId, action.param(mockDisplayPinPercentageActionPercentageParamTypeId).value().toInt());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinAllowedValuesActionTypeId) {
            device->setStateValue(mockDisplayPinAllowedValuesStateTypeId, action.param(mockDisplayPinAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinDoubleActionTypeId) {
            device->setStateValue(mockDisplayPinDoubleStateTypeId, action.param(mockDisplayPinDoubleActionDoubleParamTypeId).value().toDouble());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinBoolActionTypeId) {
            device->setStateValue(mockDisplayPinBoolStateTypeId, action.param(mockDisplayPinBoolActionBoolParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        } else if (action.actionTypeId() == mockDisplayPinTimeoutActionTypeId) {
            return Device::DeviceErrorAsync;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockParentDeviceClassId) {
        if (action.actionTypeId() == mockParentBoolValueActionTypeId) {
            device->setStateValue(mockParentBoolValueStateTypeId, action.param(mockParentBoolValueActionBoolValueParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockChildDeviceClassId) {
        if (action.actionTypeId() == mockChildBoolValueActionTypeId) {
            device->setStateValue(mockChildBoolValueStateTypeId, action.param(mockChildBoolValueActionBoolValueParamTypeId).value().toBool());
            return Device::DeviceErrorNoError;
        }
        return Device::DeviceErrorActionTypeNotFound;
    } else if (device->deviceClassId() == mockInputTypeDeviceClassId) {
        if (action.actionTypeId() == mockInputTypeWritableBoolActionTypeId) {
            device->setStateValue(mockInputTypeWritableBoolStateTypeId, action.param(mockInputTypeWritableBoolActionWritableBoolParamTypeId).value().toULongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableIntStateTypeId, action.param(mockInputTypeWritableIntActionWritableIntParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableIntMinMaxActionTypeId) {
            device->setStateValue(mockInputTypeWritableIntMinMaxStateTypeId, action.param(mockInputTypeWritableIntMinMaxActionWritableIntMinMaxParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableUIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableUIntStateTypeId, action.param(mockInputTypeWritableUIntActionWritableUIntParamTypeId).value().toULongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableUIntMinMaxActionTypeId) {
            device->setStateValue(mockInputTypeWritableUIntMinMaxStateTypeId, action.param(mockInputTypeWritableUIntMinMaxActionWritableUIntMinMaxParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableDoubleActionTypeId) {
            device->setStateValue(mockInputTypeWritableDoubleStateTypeId, action.param(mockInputTypeWritableDoubleActionWritableDoubleParamTypeId).value().toDouble());
        } else if (action.actionTypeId() == mockInputTypeWritableDoubleMinMaxActionTypeId) {
            device->setStateValue(mockInputTypeWritableDoubleMinMaxStateTypeId, action.param(mockInputTypeWritableDoubleMinMaxActionWritableDoubleMinMaxParamTypeId).value().toDouble());
        } else if (action.actionTypeId() == mockInputTypeWritableStringActionTypeId) {
            device->setStateValue(mockInputTypeWritableStringStateTypeId, action.param(mockInputTypeWritableStringActionWritableStringParamTypeId).value().toString());
        } else if (action.actionTypeId() == mockInputTypeWritableStringSelectionActionTypeId) {
            device->setStateValue(mockInputTypeWritableStringSelectionStateTypeId, action.param(mockInputTypeWritableStringSelectionActionWritableStringSelectionParamTypeId).value().toString());
        } else if (action.actionTypeId() == mockInputTypeWritableColorActionTypeId) {
            device->setStateValue(mockInputTypeWritableColorStateTypeId, action.param(mockInputTypeWritableColorActionWritableColorParamTypeId).value().toString());
        } else if (action.actionTypeId() == mockInputTypeWritableTimeActionTypeId) {
            device->setStateValue(mockInputTypeWritableTimeStateTypeId, action.param(mockInputTypeWritableTimeActionWritableTimeParamTypeId).value().toTime());
        } else if (action.actionTypeId() == mockInputTypeWritableTimestampIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableTimestampIntStateTypeId, action.param(mockInputTypeWritableTimestampIntActionWritableTimestampIntParamTypeId).value().toLongLong());
        } else if (action.actionTypeId() == mockInputTypeWritableTimestampUIntActionTypeId) {
            device->setStateValue(mockInputTypeWritableTimestampUIntStateTypeId, action.param(mockInputTypeWritableTimestampUIntActionWritableTimestampUIntParamTypeId).value().toULongLong());
        }

    }
    return Device::DeviceErrorDeviceClassNotFound;
}

Device::DeviceError DevicePluginMock::executeBrowserItem(Device *device, const BrowserAction &browserAction)
{
    qCDebug(dcMockDevice()) << "ExecuteBrowserItem called" << browserAction.itemId();
    bool broken = device->paramValue(mockDeviceBrokenParamTypeId).toBool();
    bool async = device->paramValue(mockDeviceAsyncParamTypeId).toBool();

    VirtualFsNode *node = m_virtualFs->findNode(browserAction.itemId());
    if (!node) {
        return Device::DeviceErrorItemNotFound;
    }

    if (!node->item.executable()) {
        return Device::DeviceErrorItemNotExecutable;
    }

    if (!async){
        if (broken) {
            return Device::DeviceErrorHardwareFailure;
        }
        return Device::DeviceErrorNoError;
    }

    QTimer::singleShot(2000, device, [this, broken, browserAction](){
        emit this->browserItemExecutionFinished(browserAction.id(), broken ? Device::DeviceErrorHardwareFailure : Device::DeviceErrorNoError);
    });
    return Device::DeviceErrorAsync;
}

Device::DeviceError DevicePluginMock::executeBrowserItemAction(Device *device, const BrowserItemAction &browserItemAction)
{
    qCDebug(dcMockDevice()) << "TODO" << device << browserItemAction.id();
    if (browserItemAction.actionTypeId() == mockAddToFavoritesBrowserItemActionTypeId) {

        VirtualFsNode *node = m_virtualFs->findNode(browserItemAction.itemId());
        if (!node) {
            return Device::DeviceErrorInvalidParameter;
        }
        VirtualFsNode *favoritesNode = m_virtualFs->findNode("favorites");
        if (favoritesNode->findNode(browserItemAction.itemId())) {
            return Device::DeviceErrorDeviceInUse;
        }
        BrowserItem newItem = node->item;
        newItem.setActionTypeIds({mockRemoveFromFavoritesBrowserItemActionTypeId});
        VirtualFsNode *newNode = new VirtualFsNode(newItem);
        favoritesNode->addChild(newNode);
        return Device::DeviceErrorNoError;
    }

    if (browserItemAction.actionTypeId() == mockRemoveFromFavoritesBrowserItemActionTypeId) {
        VirtualFsNode *favoritesNode = m_virtualFs->findNode("favorites");
        VirtualFsNode *nodeToRemove = favoritesNode->findNode(browserItemAction.itemId());
        if (!nodeToRemove) {
            return Device::DeviceErrorItemNotFound;
        }
        int idx = favoritesNode->childs.indexOf(nodeToRemove);
        delete favoritesNode->childs.takeAt(idx);
        return Device::DeviceErrorNoError;
    }

    return Device::DeviceErrorActionTypeNotFound;
}

void DevicePluginMock::setState(const StateTypeId &stateTypeId, const QVariant &value)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Device *device = m_daemons.key(daemon);
    device->setStateValue(stateTypeId, value);
}

void DevicePluginMock::triggerEvent(const EventTypeId &id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Device *device = m_daemons.key(daemon);

    Event event(id, device->id());

    qCDebug(dcMockDevice) << "Emitting event " << event.eventTypeId();
    emit emitEvent(event);
}

void DevicePluginMock::onDisappear()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }
    Device *device = m_daemons.key(daemon);
    qCDebug(dcMockDevice) << "Emitting autoDeviceDisappeared for device" << device->id();
    emit autoDeviceDisappeared(device->id());
}

void DevicePluginMock::onReconfigureAutoDevice()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon *>(sender());
    if (!daemon)
        return;

    Device *device = m_daemons.key(daemon);
    qCDebug(dcMockDevice()) << "Reconfigure auto device for" << device << device->params();

    int currentPort = device->params().paramValue(mockDeviceAutoDeviceHttpportParamTypeId).toInt();

    // Note: the reconfigure makes the http server listen on port + 1
    ParamList params;
    params.append(Param(mockDeviceAutoDeviceHttpportParamTypeId, currentPort + 1));

    DeviceDescriptor deviceDescriptor;
    deviceDescriptor.setTitle(device->name() + " (reconfigured)");
    deviceDescriptor.setDescription("This auto device was reconfigured");
    deviceDescriptor.setDeviceId(device->id());
    deviceDescriptor.setParams(params);

    emit autoDevicesAppeared({ deviceDescriptor });
}

void DevicePluginMock::emitDevicesDiscovered(DeviceDiscoveryInfo info)
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockDeviceClassId, "Mock Device 1 (Discovered)", "55555");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55555");
        params.append(httpParam);
        d1.setParams(params);
        foreach (Device *d, myDevices()) {
            if (d->deviceClassId() == mockDeviceClassId && d->paramValue(mockDeviceHttpportParamTypeId).toInt() == 55555) {
                d1.setDeviceId(d->id());
                break;
            }
        }
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDeviceClassId, "Mock Device 2 (Discovered)", "55556");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55556");
        params.append(httpParam);
        d2.setParams(params);
        foreach (Device *d, myDevices()) {
            if (d->deviceClassId() == mockDeviceClassId && d->paramValue(mockDeviceHttpportParamTypeId).toInt() == 55556) {
                d2.setDeviceId(d->id());
                break;
            }
        }
        deviceDescriptors.append(d2);
    }

    info.setStatus(Device::DeviceErrorNoError);
    info.setDeviceDescriptors(deviceDescriptors);
    emit devicesDiscovered(info);
}

void DevicePluginMock::emitPushButtonDevicesDiscovered(DeviceDiscoveryInfo deviceDiscoveryInfo)
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockPushButtonDeviceClassId, "Mock Device (Push Button)", "1");
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockPushButtonDeviceClassId, "Mock Device (Push Button)", "2");
        deviceDescriptors.append(d2);
    }
    deviceDiscoveryInfo.setDeviceDescriptors(deviceDescriptors);
    deviceDiscoveryInfo.setStatus(Device::DeviceErrorNoError);
    emit devicesDiscovered(deviceDiscoveryInfo);

    m_pushbuttonPressed = false;
    QTimer::singleShot(3000, this, SLOT(onPushButtonPressed()));
    qCDebug(dcMockDevice) << "Start PushButton timer (will be pressed in 3 second)";
}

void DevicePluginMock::emitDisplayPinDevicesDiscovered(DeviceDiscoveryInfo deviceDiscoveryInfo)
{
    QList<DeviceDescriptor> deviceDescriptors;

    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockDisplayPinDeviceClassId, "Mock Device (Display Pin)", "1");
        foreach (Device *existingDev, myDevices()) {
            if (existingDev->deviceClassId() == mockDisplayPinDeviceClassId) {
                d1.setDeviceId(existingDev->id());
                break;
            }
        }
        deviceDescriptors.append(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockDisplayPinDeviceClassId, "Mock Device (Display Pin)", "2");
        int count = 0;
        foreach (Device *existingDev, myDevices()) {
            if (existingDev->deviceClassId() == mockDisplayPinDeviceClassId && ++count > 1) {
                d2.setDeviceId(existingDev->id());
                break;
            }
        }
        deviceDescriptors.append(d2);
    }

    deviceDiscoveryInfo.setDeviceDescriptors(deviceDescriptors);
    deviceDiscoveryInfo.setStatus(Device::DeviceErrorNoError);
    emit devicesDiscovered(deviceDiscoveryInfo);
}

void DevicePluginMock::onPushButtonPressed()
{
    qCDebug(dcMockDevice) << "PushButton pressed (automatically)";
    m_pushbuttonPressed = true;
}

void DevicePluginMock::emitDeviceSetupFinished()
{
    qCDebug(dcMockDevice) << "Emitting setup finised";
    Device *device = m_asyncSetupDevices.takeFirst();
    if (device->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
        emit deviceSetupFinished(DeviceSetupInfo(device->id(), Device::DeviceErrorHardwareFailure, QT_TR_NOOP("Hardware is broken!")));
    } else {
        emit deviceSetupFinished(DeviceSetupInfo(device->id(), Device::DeviceErrorNoError, QT_TR_NOOP("All good.")));
    }
}

void DevicePluginMock::emitActionExecuted()
{
    QPair<Action, Device*> action = m_asyncActions.takeFirst();
    if (action.first.actionTypeId() == mockAsyncActionTypeId) {
        m_daemons.value(action.second)->actionExecuted(action.first.actionTypeId());
        emit actionExecutionFinished(action.first.id(), Device::DeviceErrorNoError);
    } else if (action.first.actionTypeId() == mockAsyncFailingActionTypeId) {
        emit actionExecutionFinished(action.first.id(), Device::DeviceErrorSetupFailed);
    }
}

void DevicePluginMock::onPushButtonPairingFinished()
{
    qCDebug(dcMockDevice) << "Pairing PushButton Device finished";
    m_pairingInfo.setStatus(Device::DeviceErrorNoError);
    emit pairingFinished(m_pairingInfo);
}

void DevicePluginMock::onDisplayPinPairingFinished()
{
    qCDebug(dcMockDevice) << "Pairing DisplayPin Device finished";
    m_pairingInfo.setStatus(Device::DeviceErrorNoError);
    emit pairingFinished(m_pairingInfo);
}

void DevicePluginMock::onChildDeviceDiscovered(const DeviceId &parentId)
{
    qCDebug(dcMockDevice) << "Child device discovered for parent" << parentId.toString();
    DeviceDescriptor mockDescriptor(mockChildDeviceClassId, "Child Mock Device (Auto created)", "Child Mock Device (Auto created)", parentId);
    emit autoDevicesAppeared({mockDescriptor});
}

void DevicePluginMock::onPluginConfigChanged()
{

}

void DevicePluginMock::generateBrowseItems()
{
    m_virtualFs = new VirtualFsNode(BrowserItem());

    BrowserItem item = BrowserItem("001", "Item 0", true);
    item.setDescription("I'm a folder");
    item.setIcon(BrowserItem::BrowserIconFolder);
    VirtualFsNode *folderNode = new VirtualFsNode(item);
    m_virtualFs->addChild(folderNode);

    item = BrowserItem("002", "Item 1", false, true);
    item.setDescription("I'm executable");
    item.setIcon(BrowserItem::BrowserIconApplication);
    item.setActionTypeIds({mockAddToFavoritesBrowserItemActionTypeId});
    m_virtualFs->addChild(new VirtualFsNode(item));

    item = BrowserItem("003", "Item 2", false, true);
    item.setDescription("I'm a file");
    item.setIcon(BrowserItem::BrowserIconFile);
    item.setActionTypeIds({mockAddToFavoritesBrowserItemActionTypeId});
    m_virtualFs->addChild(new VirtualFsNode(item));

    item = BrowserItem("004", "Item 3", false, true);
    item.setDescription("I have a nice thumbnail");
    item.setIcon(BrowserItem::BrowserIconFile);
    item.setThumbnail("https://github.com/guh/nymea/raw/master/icons/nymea-logo-256x256.png");
    item.setActionTypeIds({mockAddToFavoritesBrowserItemActionTypeId});
    m_virtualFs->addChild(new VirtualFsNode(item));

    item = BrowserItem("005", "Item 4", false, false);
    item.setDescription("I'm disabled");
    item.setDisabled(true);
    item.setIcon(BrowserItem::BrowserIconFile);
    m_virtualFs->addChild(new VirtualFsNode(item));

    item = BrowserItem("favorites", "Favorites", true, false);
    item.setDescription("Yay! I'm the best!");
    item.setIcon(BrowserItem::BrowserIconFavorites);
    m_virtualFs->addChild(new VirtualFsNode(item));

    item = BrowserItem("sub-001", "Item Subdir 1", false, true);
    item.setDescription("I'm an item in a subdir");
    item.setIcon(BrowserItem::BrowserIconFile);
    folderNode->addChild(new VirtualFsNode(item));

    item = BrowserItem("sub-002", "Item Subdir 2", true, false);
    item.setDescription("I'm a folder in a subdir");
    item.setIcon(BrowserItem::BrowserIconFile);
    folderNode->addChild(new VirtualFsNode(item));

}
