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
#include "devices/devicediscoveryinfo.h"
#include "devices/devicepairinginfo.h"
#include "devices/devicesetupinfo.h"
#include "devices/deviceactioninfo.h"
#include "devices/browseresult.h"
#include "devices/browseritemresult.h"
#include "devices/browseractioninfo.h"
#include "devices/browseritemactioninfo.h"
#include "plugininfo.h"

#include "network/networkaccessmanager.h"

#include <QDebug>
#include <QColor>
#include <QStringList>
#include <QTimer>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>

DevicePluginMock::DevicePluginMock()
{
    generateBrowseItems();
}

DevicePluginMock::~DevicePluginMock()
{
}

void DevicePluginMock::discoverDevices(DeviceDiscoveryInfo *info)
{
    if (info->deviceClassId() == mockDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info](){
            generateDiscoveredDevices(info);
        });
        return;
    }

    if (info->deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock push button discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockPushButtonDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info]() {
            generateDiscoveredPushButtonDevices(info);
        });
        return;
    }

    if (info->deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "starting mock display pin discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockDisplayPinDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info]() {
            generateDiscoveredDisplayPinDevices(info);
        });
        return;
    }

    if (info->deviceClassId() == mockParentDeviceClassId) {
        qCDebug(dcMockDevice()) << "Starting discovery for mock device parent";
        QTimer::singleShot(1000, info, [info](){
            DeviceDescriptor descriptor(mockParentDeviceClassId, "Mock Parent (Discovered)");
            info->addDeviceDescriptor(descriptor);
            info->finish(Device::DeviceErrorNoError);
        });
        return;
    }

    if (info->deviceClassId() == mockChildDeviceClassId) {
        QTimer::singleShot(1000, info, [this, info](){
            if (!myDevices().filterByDeviceClassId(mockParentDeviceClassId).isEmpty()) {
                Device *parent = myDevices().filterByDeviceClassId(mockParentDeviceClassId).first();
                DeviceDescriptor descriptor(mockChildDeviceClassId, "Mock Child (Discovered)", QString(), parent->id());
                info->addDeviceDescriptor(descriptor);
            }
            info->finish(Device::DeviceErrorNoError);
        });
        return;
    }

    qCWarning(dcMockDevice()) << "Cannot discover for deviceClassId" << info->deviceClassId();
    info->finish(Device::DeviceErrorDeviceNotFound);
}

void DevicePluginMock::setupDevice(DeviceSetupInfo *info)
{
    if (info->device()->deviceClassId() == mockDeviceClassId || info->device()->deviceClassId() == mockDeviceAutoDeviceClassId) {
        bool async = false;
        bool broken = false;
        if (info->device()->deviceClassId() == mockDeviceClassId) {
            async = info->device()->paramValue(mockDeviceAsyncParamTypeId).toBool();
            broken = info->device()->paramValue(mockDeviceBrokenParamTypeId).toBool();
        } else {
            async = info->device()->paramValue(mockDeviceAutoDeviceAsyncParamTypeId).toBool();
            broken = info->device()->paramValue(mockDeviceAutoDeviceBrokenParamTypeId).toBool();
        }

        if (!async && broken) {
            qCWarning(dcMockDevice) << "This device is intentionally broken.";
            info->finish(Device::DeviceErrorSetupFailed, QT_TR_NOOP("This mock device is intentionally broken."));
            return;
        }

        if (!broken) {
            HttpDaemon *daemon = new HttpDaemon(info->device(), this);
            m_daemons.insert(info->device(), daemon);

            if (!daemon->isListening()) {
                qCWarning(dcMockDevice) << "HTTP port opening failed:" << info->device()->paramValue(mockDeviceHttpportParamTypeId).toInt();
                info->finish(Device::DeviceErrorHardwareNotAvailable, QT_TR_NOOP("Failed to open HTTP port. Port in use?"));
                return;
            }

            connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
            connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);
            // Keep this queued or it might happen that the HttpDaemon is deleted before it is able to reply to the caller
            connect(daemon, &HttpDaemon::disappear, this, &DevicePluginMock::onDisappear, Qt::QueuedConnection);
            connect(daemon, &HttpDaemon::reconfigureAutodevice, this, &DevicePluginMock::onReconfigureAutoDevice, Qt::QueuedConnection);
        }


        if (async) {
            Device *device = info->device();
            QTimer::singleShot(1000, device, [info](){
                qCDebug(dcMockDevice) << "Finishing device setup for mock device" << info->device()->name();
                if (info->device()->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
                    info->finish(Device::DeviceErrorSetupFailed, QT_TR_NOOP("This mock device is intentionally broken."));
                } else {
                    info->finish(Device::DeviceErrorNoError);
                }
            });
            return;
        }
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup PushButton mock device" << info->device()->params();
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup DisplayPin mock device" << info->device()->params();
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockParentDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Parent mock device" << info->device()->params();
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockChildDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup Child mock device" << info->device()->params();
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockInputTypeDeviceClassId) {
        qCDebug(dcMockDevice) << "Setup InputType mock device" << info->device()->params();
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockUserAndPassDeviceClassId) {
        qCDebug(dcMockDevice()) << "Setup User and password mock device";
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockOAuthGoogleDeviceClassId) {
        qCDebug(dcMockDevice()) << "Google OAuth setup complete";
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockOAuthSonosDeviceClassId) {
        qCDebug(dcMockDevice()) << "Sonos OAuth setup complete";
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    qCWarning(dcMockDevice()) << "Unhandled device class" << info->device()->deviceClass();
    info->finish(Device::DeviceErrorDeviceClassNotFound);
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

        DeviceDescriptor mockDescriptor(mockChildDeviceClassId, "Child Mock Device (Auto created)", "Child Mock Device (Auto created)", device->id());
        emit autoDevicesAppeared(DeviceDescriptors() << mockDescriptor);
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

void DevicePluginMock::startPairing(DevicePairingInfo *info)
{
    if (info->deviceClassId() == mockPushButtonDeviceClassId) {
        qCDebug(dcMockDevice) << QString(tr("Push button. Pressing the button in 3 seconds."));
        info->finish(Device::DeviceErrorNoError, QT_TR_NOOP("Wait 3 second before you continue, the push button will be pressed automatically."));
        return;
    }

    if (info->deviceClassId() == mockDisplayPinDeviceClassId) {
        qCDebug(dcMockDevice) << QString(tr("Display pin!! The pin is 243681"));
        info->finish(Device::DeviceErrorNoError, QT_TR_NOOP("Please enter the secret which normaly will be displayed on the device. For the mockdevice the pin is 243681."));
        return;
    }

    if (info->deviceClassId() == mockUserAndPassDeviceClassId) {
        qCDebug(dcMockDevice) << QString(tr("User and password. Login is \"user\" and \"password\"."));
        info->finish(Device::DeviceErrorNoError, QT_TR_NOOP("Please enter login credentials for the mock device (\"user\" and \"password\")."));
        return;
    }

    if (info->deviceClassId() == mockOAuthSonosDeviceClassId) {
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

        info->setOAuthUrl(url);
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->deviceClassId() == mockOAuthGoogleDeviceClassId) {
        QString clientId= "937667874529-pr6s5ciu6sfnnqmt2sppvb6rokbkjjta.apps.googleusercontent.com";
        QString clientSecret = "1ByBRmNqaK08VC54eEVcnGf1";

        QUrl url("https://accounts.google.com/o/oauth2/v2/auth");
        QUrlQuery queryParams;
        queryParams.addQueryItem("client_id", clientId);
        queryParams.addQueryItem("redirect_uri", "https://127.0.0.1:8888");
        queryParams.addQueryItem("response_type", "code");
        queryParams.addQueryItem("scope", "profile email");
        queryParams.addQueryItem("state", "ya-ya");
        url.setQuery(queryParams);

        info->setOAuthUrl(url);
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    info->finish(Device::DeviceErrorCreationMethodNotSupported);
}

void DevicePluginMock::confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret)
{
    qCDebug(dcMockDevice) << "Confirm pairing";

    if (info->deviceClassId() == mockPushButtonDeviceClassId) {
        if (!m_pushbuttonPressed) {
            qCDebug(dcMockDevice) << "PushButton not pressed yet!";
            info->finish(Device::DeviceErrorAuthenticationFailure, QT_TR_NOOP("The push button has not been pressed."));
            return;
        }

        QTimer::singleShot(1000, this, [info](){
            info->finish(Device::DeviceErrorNoError);
        });
        return;
    }

    if (info->deviceClassId() == mockDisplayPinDeviceClassId) {
        if (secret != "243681") {
            qCWarning(dcMockDevice) << "Invalid pin:" << secret;
            info->finish(Device::DeviceErrorAuthenticationFailure, QT_TR_NOOP("Invalid PIN!"));
            return;
        }
        QTimer::singleShot(500, this, [info](){
            qCDebug(dcMockDevice()) << "Pairing finished.";
            info->finish(Device::DeviceErrorNoError);
        });
        return;
    }

    if (info->deviceClassId() == mockUserAndPassDeviceClassId) {
        qCDebug(dcMockDevice()) << "Credentials received:" << username << secret;
        if (username == "user" && secret == "password") {
            info->finish(Device::DeviceErrorNoError);
            return;
        } else {
            info->finish(Device::DeviceErrorAuthenticationFailure, QT_TR_NOOP("Wrong username or password"));
            return;
        }
    }


    if (info->deviceClassId() == mockOAuthSonosDeviceClassId) {
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
        connect(reply, &QNetworkReply::finished, this, [this, reply, info](){
            reply->deleteLater();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            qCDebug(dcMockDevice()) << "Sonos accessToken reply:" << this << reply->error() << reply->errorString() << jsonDoc.toJson();
            qCDebug(dcMockDevice()) << "Access token:" << jsonDoc.toVariant().toMap().value("access_token").toString();
            qCDebug(dcMockDevice()) << "expires at" << QDateTime::currentDateTime().addSecs(jsonDoc.toVariant().toMap().value("expires_in").toInt()).toString();
            qCDebug(dcMockDevice()) << "Refresh token:" << jsonDoc.toVariant().toMap().value("refresh_token").toString();
            info->finish(Device::DeviceErrorNoError);
        });

        return;
    }


    if (info->deviceClassId() == mockOAuthGoogleDeviceClassId) {
        qCDebug(dcMockDevice()) << "Secret is" << secret;
        QUrl url(secret);
        QUrlQuery query(url);
        qCDebug(dcMockDevice()) << "Acess code is:" << query.queryItemValue("code");

        QString accessCode = query.queryItemValue("code");

        // Obtaining access token
        QString clientId = "937667874529-pr6s5ciu6sfnnqmt2sppvb6rokbkjjta.apps.googleusercontent.com";
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
        connect(reply, &QNetworkReply::finished, this, [this, reply, info](){
            reply->deleteLater();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            qCDebug(dcMockDevice()) << "Sonos accessToken reply:" << this << reply->error() << reply->errorString() << jsonDoc.toJson();
            qCDebug(dcMockDevice()) << "Access token:" << jsonDoc.toVariant().toMap().value("access_token").toString();
            qCDebug(dcMockDevice()) << "expires at" << QDateTime::currentDateTime().addSecs(jsonDoc.toVariant().toMap().value("expires_in").toInt()).toString();
            qCDebug(dcMockDevice()) << "Refresh token:" << jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcMockDevice()) << "ID token:" << jsonDoc.toVariant().toMap().value("id_token").toString();
            info->finish(Device::DeviceErrorNoError);
        });
        return;
    }


    qCWarning(dcMockDevice) << "Invalid deviceclassId -> no pairing possible with this device";
    info->finish(Device::DeviceErrorDeviceClassNotFound);
}

void DevicePluginMock::browseDevice(BrowseResult *result)
{
    qCDebug(dcMockDevice()) << "Browse device called" << result->device();
    if (result->device()->deviceClassId() == mockDeviceClassId) {
        if (result->device()->paramValue(mockDeviceAsyncParamTypeId).toBool()) {

            QTimer::singleShot(1000, result, [this, result]() {
                if (result->device()->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
                    result->finish(Device::DeviceErrorHardwareFailure);
                    return;
                }

                VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
                if (!node) {
                    result->finish(Device::DeviceErrorItemNotFound);
                    return;
                }

                foreach (VirtualFsNode *child, node->childs) {
                    result->addItem(child->item);
                }

                result->finish(Device::DeviceErrorNoError);
            });

            return;
        }

        if (result->device()->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
            result->finish(Device::DeviceErrorHardwareFailure);
            return;
        }

        VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
        if (!node) {
            result->finish(Device::DeviceErrorItemNotFound);
            return;
        }

        foreach (VirtualFsNode *child, node->childs) {
            result->addItem(child->item);
        }
        result->finish(Device::DeviceErrorNoError);
        return;
    }
    result->finish(Device::DeviceErrorInvalidParameter);
}

void DevicePluginMock::browserItem(BrowserItemResult *result)
{
    VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
    if (!node) {
        result->finish(Device::DeviceErrorItemNotFound);
        return;
    }
    result->finish(node->item);
}

void DevicePluginMock::executeAction(DeviceActionInfo *info)
{
    if (info->device()->deviceClassId() == mockDeviceClassId) {
        if (info->action().actionTypeId() == mockAsyncActionTypeId || info->action().actionTypeId() == mockAsyncFailingActionTypeId) {
            QTimer::singleShot(1000, info->device(), [this, info](){
                if (info->action().actionTypeId() == mockAsyncActionTypeId) {
                    m_daemons.value(info->device())->actionExecuted(info->action().actionTypeId());
                    info->finish(Device::DeviceErrorNoError);
                } else if (info->action().actionTypeId() == mockAsyncFailingActionTypeId) {
                    info->finish(Device::DeviceErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
                }

            });
            return;
        }

        if (info->action().actionTypeId() == mockFailingActionTypeId) {
            info->finish(Device::DeviceErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
            return;
        }

        if (info->action().actionTypeId() == mockPowerActionTypeId) {
            qCDebug(dcMockDevice()) << "Setting power to" << info->action().param(mockPowerActionPowerParamTypeId).value().toBool();
            info->device()->setStateValue(mockPowerStateTypeId, info->action().param(mockPowerActionPowerParamTypeId).value().toBool());
        }
        m_daemons.value(info->device())->actionExecuted(info->action().actionTypeId());
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->device()->deviceClassId() == mockDeviceAutoDeviceClassId) {
        if (info->action().actionTypeId() == mockDeviceAutoMockActionAsyncActionTypeId || info->action().actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
            QTimer::singleShot(1000, info->device(), [info](){
                if (info->action().actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
                    info->finish(Device::DeviceErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
                } else {
                    info->finish(Device::DeviceErrorNoError);
                }
            });
        }

        if (info->action().actionTypeId() == mockDeviceAutoMockActionBrokenActionTypeId) {
            info->finish(Device::DeviceErrorSetupFailed);
            return;
        }

        m_daemons.value(info->device())->actionExecuted(info->action().actionTypeId());
        info->finish(Device::DeviceErrorNoError);
    } else if (info->device()->deviceClassId() == mockPushButtonDeviceClassId) {
        if (info->action().actionTypeId() == mockPushButtonColorActionTypeId) {
            QString colorString = info->action().param(mockPushButtonColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                info->finish(Device::DeviceErrorInvalidParameter);
                return;
            }
            info->device()->setStateValue(mockPushButtonColorStateTypeId, colorString);
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonPercentageActionTypeId) {
            info->device()->setStateValue(mockPushButtonPercentageStateTypeId, info->action().param(mockPushButtonPercentageActionPercentageParamTypeId).value().toInt());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonAllowedValuesActionTypeId) {
            info->device()->setStateValue(mockPushButtonAllowedValuesStateTypeId, info->action().param(mockPushButtonAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonDoubleActionTypeId) {
            info->device()->setStateValue(mockPushButtonDoubleStateTypeId, info->action().param(mockPushButtonDoubleActionDoubleParamTypeId).value().toDouble());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonBoolActionTypeId) {
            info->device()->setStateValue(mockPushButtonBoolStateTypeId, info->action().param(mockPushButtonBoolActionBoolParamTypeId).value().toBool());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonTimeoutActionTypeId) {
            // Not finishing action intentionally...
            return;
        }
        info->finish(Device::DeviceErrorActionTypeNotFound);
        return;
    } else if (info->device()->deviceClassId() == mockDisplayPinDeviceClassId) {
        if (info->action().actionTypeId() == mockDisplayPinColorActionTypeId) {
            QString colorString = info->action().param(mockDisplayPinColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                info->finish(Device::DeviceErrorInvalidParameter);
                return;
            }
            info->device()->setStateValue(mockDisplayPinColorStateTypeId, colorString);
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinPercentageActionTypeId) {
            info->device()->setStateValue(mockDisplayPinPercentageStateTypeId, info->action().param(mockDisplayPinPercentageActionPercentageParamTypeId).value().toInt());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinAllowedValuesActionTypeId) {
            info->device()->setStateValue(mockDisplayPinAllowedValuesStateTypeId, info->action().param(mockDisplayPinAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinDoubleActionTypeId) {
            info->device()->setStateValue(mockDisplayPinDoubleStateTypeId, info->action().param(mockDisplayPinDoubleActionDoubleParamTypeId).value().toDouble());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinBoolActionTypeId) {
            info->device()->setStateValue(mockDisplayPinBoolStateTypeId, info->action().param(mockDisplayPinBoolActionBoolParamTypeId).value().toBool());
            info->finish(Device::DeviceErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinTimeoutActionTypeId) {
            // Not finishing action intentionally...
            return;
        }
        info->finish(Device::DeviceErrorActionTypeNotFound);
        return;
    } else if (info->device()->deviceClassId() == mockParentDeviceClassId) {
        if (info->action().actionTypeId() == mockParentBoolValueActionTypeId) {
            info->device()->setStateValue(mockParentBoolValueStateTypeId, info->action().param(mockParentBoolValueActionBoolValueParamTypeId).value().toBool());
            info->finish(Device::DeviceErrorNoError);
            return;
        }
        info->finish(Device::DeviceErrorActionTypeNotFound);
        return;
    } else if (info->device()->deviceClassId() == mockChildDeviceClassId) {
        if (info->action().actionTypeId() == mockChildBoolValueActionTypeId) {
            info->device()->setStateValue(mockChildBoolValueStateTypeId, info->action().param(mockChildBoolValueActionBoolValueParamTypeId).value().toBool());
            info->finish(Device::DeviceErrorNoError);
            return;
        }
        info->finish(Device::DeviceErrorActionTypeNotFound);
        return;
    } else if (info->device()->deviceClassId() == mockInputTypeDeviceClassId) {
        if (info->action().actionTypeId() == mockInputTypeWritableBoolActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableBoolStateTypeId, info->action().param(mockInputTypeWritableBoolActionWritableBoolParamTypeId).value().toULongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableIntActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableIntStateTypeId, info->action().param(mockInputTypeWritableIntActionWritableIntParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableIntMinMaxActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableIntMinMaxStateTypeId, info->action().param(mockInputTypeWritableIntMinMaxActionWritableIntMinMaxParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableUIntActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableUIntStateTypeId, info->action().param(mockInputTypeWritableUIntActionWritableUIntParamTypeId).value().toULongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableUIntMinMaxActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableUIntMinMaxStateTypeId, info->action().param(mockInputTypeWritableUIntMinMaxActionWritableUIntMinMaxParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableDoubleActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableDoubleStateTypeId, info->action().param(mockInputTypeWritableDoubleActionWritableDoubleParamTypeId).value().toDouble());
        } else if (info->action().actionTypeId() == mockInputTypeWritableDoubleMinMaxActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableDoubleMinMaxStateTypeId, info->action().param(mockInputTypeWritableDoubleMinMaxActionWritableDoubleMinMaxParamTypeId).value().toDouble());
        } else if (info->action().actionTypeId() == mockInputTypeWritableStringActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableStringStateTypeId, info->action().param(mockInputTypeWritableStringActionWritableStringParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == mockInputTypeWritableStringSelectionActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableStringSelectionStateTypeId, info->action().param(mockInputTypeWritableStringSelectionActionWritableStringSelectionParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == mockInputTypeWritableColorActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableColorStateTypeId, info->action().param(mockInputTypeWritableColorActionWritableColorParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == mockInputTypeWritableTimeActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableTimeStateTypeId, info->action().param(mockInputTypeWritableTimeActionWritableTimeParamTypeId).value().toTime());
        } else if (info->action().actionTypeId() == mockInputTypeWritableTimestampIntActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableTimestampIntStateTypeId, info->action().param(mockInputTypeWritableTimestampIntActionWritableTimestampIntParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableTimestampUIntActionTypeId) {
            info->device()->setStateValue(mockInputTypeWritableTimestampUIntStateTypeId, info->action().param(mockInputTypeWritableTimestampUIntActionWritableTimestampUIntParamTypeId).value().toULongLong());
        }
        return;

    }
    info->finish(Device::DeviceErrorDeviceClassNotFound);
}

void DevicePluginMock::executeBrowserItem(BrowserActionInfo *info)
{
    qCDebug(dcMockDevice()) << "ExecuteBrowserItem called" << info->browserAction().itemId();
    bool broken = info->device()->paramValue(mockDeviceBrokenParamTypeId).toBool();
    bool async = info->device()->paramValue(mockDeviceAsyncParamTypeId).toBool();

    VirtualFsNode *node = m_virtualFs->findNode(info->browserAction().itemId());
    if (!node) {
        info->finish(Device::DeviceErrorItemNotFound);
        return;
    }

    if (!node->item.executable()) {
        info->finish(Device::DeviceErrorItemNotExecutable);
        return;
    }

    if (!async){
        if (broken) {
            info->finish(Device::DeviceErrorHardwareFailure);
            return;
        }
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    QTimer::singleShot(2000, info, [broken, info](){
        info->finish(broken ? Device::DeviceErrorHardwareFailure : Device::DeviceErrorNoError);
    });

}

void DevicePluginMock::executeBrowserItemAction(BrowserItemActionInfo *info)
{
    qCDebug(dcMockDevice()) << "TODO" << info << info->browserItemAction().id();
    if (info->browserItemAction().actionTypeId() == mockAddToFavoritesBrowserItemActionTypeId) {

        VirtualFsNode *node = m_virtualFs->findNode(info->browserItemAction().itemId());
        if (!node) {
            info->finish(Device::DeviceErrorInvalidParameter);
            return;
        }

        VirtualFsNode *favoritesNode = m_virtualFs->findNode("favorites");
        if (favoritesNode->findNode(info->browserItemAction().itemId())) {
            info->finish(Device::DeviceErrorDeviceInUse);
            return;
        }
        BrowserItem newItem = node->item;
        newItem.setActionTypeIds({mockRemoveFromFavoritesBrowserItemActionTypeId});
        VirtualFsNode *newNode = new VirtualFsNode(newItem);
        favoritesNode->addChild(newNode);
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    if (info->browserItemAction().actionTypeId() == mockRemoveFromFavoritesBrowserItemActionTypeId) {
        VirtualFsNode *favoritesNode = m_virtualFs->findNode("favorites");
        VirtualFsNode *nodeToRemove = favoritesNode->findNode(info->browserItemAction().itemId());
        if (!nodeToRemove) {
            info->finish(Device::DeviceErrorItemNotFound);
            return;
        }
        int idx = favoritesNode->childs.indexOf(nodeToRemove);
        delete favoritesNode->childs.takeAt(idx);
        info->finish(Device::DeviceErrorNoError);
        return;
    }

    info->finish(Device::DeviceErrorActionTypeNotFound);
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

    DeviceDescriptor deviceDescriptor(mockDeviceAutoDeviceClassId);
    deviceDescriptor.setTitle(device->name() + " (reconfigured)");
    deviceDescriptor.setDescription("This auto device was reconfigured");
    deviceDescriptor.setDeviceId(device->id());
    deviceDescriptor.setParams(params);

    emit autoDevicesAppeared({deviceDescriptor});
}

void DevicePluginMock::generateDiscoveredDevices(DeviceDiscoveryInfo *info)
{
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
        info->addDeviceDescriptor(d1);
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
        info->addDeviceDescriptor(d2);
    }

    info->finish(Device::DeviceErrorNoError);
}

void DevicePluginMock::generateDiscoveredPushButtonDevices(DeviceDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockPushButtonDeviceClassId, "Mock Device (Push Button)", "1");
        info->addDeviceDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        DeviceDescriptor d2(mockPushButtonDeviceClassId, "Mock Device (Push Button)", "2");
        info->addDeviceDescriptor(d2);
    }
    info->finish(Device::DeviceErrorNoError, QT_TR_NOOP("This device will simulate a push button press in 3 seconds."));

    m_pushbuttonPressed = false;
    QTimer::singleShot(3000, this, SLOT(onPushButtonPressed()));
    qCDebug(dcMockDevice) << "Start PushButton timer (will be pressed in 3 second)";
}

void DevicePluginMock::generateDiscoveredDisplayPinDevices(DeviceDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        DeviceDescriptor d1(mockDisplayPinDeviceClassId, "Mock Device (Display Pin)", "1");
        foreach (Device *existingDev, myDevices()) {
            if (existingDev->deviceClassId() == mockDisplayPinDeviceClassId) {
                d1.setDeviceId(existingDev->id());
                break;
            }
        }
        info->addDeviceDescriptor(d1);
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
        info->addDeviceDescriptor(d2);
    }

    info->finish(Device::DeviceErrorNoError);
}

void DevicePluginMock::onPushButtonPressed()
{
    qCDebug(dcMockDevice) << "PushButton pressed (automatically)";
    m_pushbuttonPressed = true;
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
