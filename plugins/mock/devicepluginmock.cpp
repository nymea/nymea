/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

#include "types/mediabrowseritem.h"
#include "integrations/thing.h"
#include "integrations/thingdiscoveryinfo.h"
#include "integrations/thingpairinginfo.h"
#include "integrations/thingsetupinfo.h"
#include "integrations/thingactioninfo.h"
#include "integrations/browseresult.h"
#include "integrations/browseritemresult.h"
#include "integrations/browseractioninfo.h"
#include "integrations/browseritemactioninfo.h"
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
    delete m_virtualFs;
}

void DevicePluginMock::discoverThings(ThingDiscoveryInfo *info)
{
    if (info->thingClassId() == mockThingClassId) {
        qCDebug(dcMockDevice) << "starting mock discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info](){
            generateDiscoveredDevices(info);
        });
        return;
    }

    if (info->thingClassId() == mockPushButtonThingClassId) {
        qCDebug(dcMockDevice) << "starting mock push button discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockPushButtonDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info]() {
            generateDiscoveredPushButtonDevices(info);
        });
        return;
    }

    if (info->thingClassId() == mockDisplayPinThingClassId) {
        qCDebug(dcMockDevice) << "starting mock display pin discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockDisplayPinDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info]() {
            generateDiscoveredDisplayPinDevices(info);
        });
        return;
    }

    if (info->thingClassId() == mockParentThingClassId) {
        qCDebug(dcMockDevice()) << "Starting discovery for mock device parent";
        QTimer::singleShot(1000, info, [info](){
            ThingDescriptor descriptor(mockParentThingClassId, "Mock Parent (Discovered)");
            info->addThingDescriptor(descriptor);
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == mockChildThingClassId) {
        QTimer::singleShot(1000, info, [this, info](){
            if (!myThings().filterByThingClassId(mockParentThingClassId).isEmpty()) {
                Thing *parent = myThings().filterByThingClassId(mockParentThingClassId).first();
                ThingDescriptor descriptor(mockChildThingClassId, "Mock Child (Discovered)", QString(), parent->id());
                info->addThingDescriptor(descriptor);
            }
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == mockUserAndPassThingClassId) {
        QTimer::singleShot(1000, info, [this, info](){
            if (myThings().filterByThingClassId(mockUserAndPassThingClassId).isEmpty()) {
                ThingDescriptor descriptor(mockUserAndPassThingClassId, "Mock User & Password (Discovered)", QString());
                info->addThingDescriptor(descriptor);
            }
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    qCWarning(dcMockDevice()) << "Cannot discover for ThingClassId" << info->thingClassId();
    info->finish(Thing::ThingErrorThingNotFound);
}

void DevicePluginMock::setupThing(ThingSetupInfo *info)
{
    if (info->thing()->thingClassId() == mockThingClassId || info->thing()->thingClassId() == mockDeviceAutoThingClassId) {
        bool async = false;
        bool broken = false;
        if (info->thing()->thingClassId() == mockThingClassId) {
            async = info->thing()->paramValue(mockDeviceAsyncParamTypeId).toBool();
            broken = info->thing()->paramValue(mockDeviceBrokenParamTypeId).toBool();
        } else {
            async = info->thing()->paramValue(mockDeviceAutoDeviceAsyncParamTypeId).toBool();
            broken = info->thing()->paramValue(mockDeviceAutoDeviceBrokenParamTypeId).toBool();
        }
        qCDebug(dcMockDevice()) << "SetupDevice for" << info->thing()->name() << "Async:" << async << "Broken:" << broken;

        if (!async && broken) {
            qCWarning(dcMockDevice) << "This device is intentionally broken.";
            info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mock device is intentionally broken."));
            return;
        }

        if (!broken) {
            HttpDaemon *daemon = new HttpDaemon(info->thing(), this);
            m_daemons.insert(info->thing(), daemon);

            if (!daemon->isListening()) {
                qCWarning(dcMockDevice) << "HTTP port opening failed:" << info->thing()->paramValue(mockDeviceHttpportParamTypeId).toInt();
                info->finish(Thing::ThingErrorHardwareNotAvailable, QT_TR_NOOP("Failed to open HTTP port. Port in use?"));
                return;
            }

            connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
            connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);
            // Keep this queued or it might happen that the HttpDaemon is deleted before it is able to reply to the caller
            connect(daemon, &HttpDaemon::disappear, this, &DevicePluginMock::onDisappear, Qt::QueuedConnection);
            connect(daemon, &HttpDaemon::reconfigureAutodevice, this, &DevicePluginMock::onReconfigureAutoDevice, Qt::QueuedConnection);
        }


        if (async) {
            Thing *device = info->thing();
            QTimer::singleShot(1000, device, [info](){
                qCDebug(dcMockDevice) << "Finishing device setup for mock device" << info->thing()->name();
                if (info->thing()->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
                    info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mock device is intentionally broken."));
                } else {
                    info->finish(Thing::ThingErrorNoError);
                }
            });
            return;
        }
        qCDebug(dcMockDevice()) << "Setup complete" << info->thing()->name();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockPushButtonThingClassId) {
        qCDebug(dcMockDevice) << "Setup PushButton mock device" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockDisplayPinThingClassId) {
        qCDebug(dcMockDevice) << "Setup DisplayPin mock device" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockParentThingClassId) {
        qCDebug(dcMockDevice) << "Setup Parent mock device" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockChildThingClassId) {
        qCDebug(dcMockDevice) << "Setup Child mock device" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockInputTypeThingClassId) {
        qCDebug(dcMockDevice) << "Setup InputType mock device" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockUserAndPassThingClassId) {
        qCDebug(dcMockDevice()) << "Setup User and password mock device";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockOAuthGoogleThingClassId) {
        qCDebug(dcMockDevice()) << "Google OAuth setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockOAuthSonosThingClassId) {
        qCDebug(dcMockDevice()) << "Sonos OAuth setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    qCWarning(dcMockDevice()) << "Unhandled device class" << info->thing()->thingClass();
    info->finish(Thing::ThingErrorThingClassNotFound);
}

void DevicePluginMock::postSetupThing(Thing *device)
{
    qCDebug(dcMockDevice) << "Postsetup mockdevice" << device->name();
    if (device->thingClassId() == mockParentThingClassId) {
        foreach (Thing *d, myThings()) {
            if (d->thingClassId() == mockChildThingClassId && d->parentId() == device->id()) {
                return;
            }
        }

        ThingDescriptor mockDescriptor(mockChildThingClassId, "Child Mock Device (Auto created)", "Child Mock Device (Auto created)", device->id());
        emit autoThingsAppeared(ThingDescriptors() << mockDescriptor);
    }
}

void DevicePluginMock::thingRemoved(Thing *device)
{
    delete m_daemons.take(device);
}

void DevicePluginMock::startMonitoringAutoThings()
{
    foreach (Thing *device, myThings()) {
        if (device->thingClassId() == mockDeviceAutoThingClassId) {
            return; // We already have a Auto Mock device... do nothing.
        }
    }

    ThingDescriptor mockDescriptor(mockDeviceAutoThingClassId, "Mock Device (Auto created)");

    ParamList params;
    qsrand(QDateTime::currentMSecsSinceEpoch());
    int port = 4242 + (qrand() % 1000);
    Param param(mockDeviceAutoDeviceHttpportParamTypeId, port);
    params.append(param);
    mockDescriptor.setParams(params);

    QList<ThingDescriptor> deviceDescriptorList;
    deviceDescriptorList.append(mockDescriptor);

    emit autoThingsAppeared(deviceDescriptorList);
}

void DevicePluginMock::startPairing(ThingPairingInfo *info)
{
    if (info->thingClassId() == mockPushButtonThingClassId) {
        qCDebug(dcMockDevice) << "Push button. Pressing the button in 3 seconds.";
        info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("Wait 3 second before you continue, the push button will be pressed automatically."));
        m_pushbuttonPressed = false;
        QTimer::singleShot(3000, this, SLOT(onPushButtonPressed()));
        return;
    }

    if (info->thingClassId() == mockDisplayPinThingClassId) {
        qCDebug(dcMockDevice) << "Display pin!! The pin is 243681";
        info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("Please enter the secret which normaly will be displayed on the device. For the mockdevice the pin is 243681."));
        return;
    }

    if (info->thingClassId() == mockUserAndPassThingClassId) {
        qCDebug(dcMockDevice) << "User and password. Login is \"user\" and \"password\".";
        info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("Please enter login credentials for the mock device (\"user\" and \"password\")."));
        return;
    }

    if (info->thingClassId() == mockOAuthSonosThingClassId) {
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
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thingClassId() == mockOAuthGoogleThingClassId) {
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
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    info->finish(Thing::ThingErrorCreationMethodNotSupported);
}

void DevicePluginMock::confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret)
{
    qCDebug(dcMockDevice) << "Confirm pairing";

    if (info->thingClassId() == mockPushButtonThingClassId) {
        if (!m_pushbuttonPressed) {
            qCDebug(dcMockDevice) << "PushButton not pressed yet!";
            info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("The push button has not been pressed."));
            return;
        }

        QTimer::singleShot(1000, this, [info](){
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == mockDisplayPinThingClassId) {
        if (secret != "243681") {
            qCWarning(dcMockDevice) << "Invalid pin:" << secret;
            info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("Invalid PIN!"));
            return;
        }
        QTimer::singleShot(500, this, [info](){
            qCDebug(dcMockDevice()) << "Pairing finished.";
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == mockUserAndPassThingClassId) {
        qCDebug(dcMockDevice()) << "Credentials received:" << username << secret;
        if (username == "user" && secret == "password") {
            info->finish(Thing::ThingErrorNoError);
            return;
        } else {
            info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("Wrong username or password"));
            return;
        }
    }


    if (info->thingClassId() == mockOAuthSonosThingClassId) {
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
        query.addQueryItem("redirect_uri", QByteArray("https://127.0.0.1:8888").toPercentEncoding());
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
            info->finish(Thing::ThingErrorNoError);
        });

        return;
    }


    if (info->thingClassId() == mockOAuthGoogleThingClassId) {
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
        query.addQueryItem("redirect_uri", QByteArray("https://127.0.0.1:8888").toPercentEncoding());
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
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }


    qCWarning(dcMockDevice) << "Invalid ThingClassId -> no pairing possible with this device";
    info->finish(Thing::ThingErrorThingClassNotFound);
}

void DevicePluginMock::browseThing(BrowseResult *result)
{
    qCDebug(dcMockDevice()) << "Browse device called" << result->thing();
    if (result->thing()->thingClassId() == mockThingClassId) {
        if (result->thing()->paramValue(mockDeviceAsyncParamTypeId).toBool()) {

            QTimer::singleShot(1000, result, [this, result]() {
                if (result->thing()->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
                    result->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
                if (!node) {
                    result->finish(Thing::ThingErrorItemNotFound);
                    return;
                }

                foreach (VirtualFsNode *child, node->childs) {
                    result->addItem(child->item);
                }

                result->finish(Thing::ThingErrorNoError);
            });

            return;
        }

        if (result->thing()->paramValue(mockDeviceBrokenParamTypeId).toBool()) {
            result->finish(Thing::ThingErrorHardwareFailure);
            return;
        }

        VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
        if (!node) {
            result->finish(Thing::ThingErrorItemNotFound);
            return;
        }

        foreach (VirtualFsNode *child, node->childs) {
            result->addItem(child->item);
        }
        result->finish(Thing::ThingErrorNoError);
        return;
    }
    result->finish(Thing::ThingErrorInvalidParameter);
}

void DevicePluginMock::browserItem(BrowserItemResult *result)
{
    VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
    if (!node) {
        result->finish(Thing::ThingErrorItemNotFound);
        return;
    }
    result->finish(node->item);
}

void DevicePluginMock::executeAction(ThingActionInfo *info)
{
    if (info->thing()->thingClassId() == mockThingClassId) {
        if (info->action().actionTypeId() == mockAsyncActionTypeId || info->action().actionTypeId() == mockAsyncFailingActionTypeId) {
            QTimer::singleShot(1000, info->thing(), [this, info](){
                if (info->action().actionTypeId() == mockAsyncActionTypeId) {
                    m_daemons.value(info->thing())->actionExecuted(info->action().actionTypeId());
                    info->finish(Thing::ThingErrorNoError);
                } else if (info->action().actionTypeId() == mockAsyncFailingActionTypeId) {
                    info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
                }

            });
            return;
        }

        if (info->action().actionTypeId() == mockFailingActionTypeId) {
            info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
            return;
        }

        if (info->action().actionTypeId() == mockPowerActionTypeId) {
            qCDebug(dcMockDevice()) << "Setting power to" << info->action().param(mockPowerActionPowerParamTypeId).value().toBool();
            info->thing()->setStateValue(mockPowerStateTypeId, info->action().param(mockPowerActionPowerParamTypeId).value().toBool());
        }
        m_daemons.value(info->thing())->actionExecuted(info->action().actionTypeId());
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == mockDeviceAutoThingClassId) {
        if (info->action().actionTypeId() == mockDeviceAutoMockActionAsyncActionTypeId || info->action().actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
            QTimer::singleShot(1000, info->thing(), [info](){
                if (info->action().actionTypeId() == mockDeviceAutoMockActionAsyncBrokenActionTypeId) {
                    info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
                } else {
                    info->finish(Thing::ThingErrorNoError);
                }
            });
        }

        if (info->action().actionTypeId() == mockDeviceAutoMockActionBrokenActionTypeId) {
            info->finish(Thing::ThingErrorSetupFailed);
            return;
        }

        m_daemons.value(info->thing())->actionExecuted(info->action().actionTypeId());
        info->finish(Thing::ThingErrorNoError);
    } else if (info->thing()->thingClassId() == mockPushButtonThingClassId) {
        if (info->action().actionTypeId() == mockPushButtonColorActionTypeId) {
            QString colorString = info->action().param(mockPushButtonColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                info->finish(Thing::ThingErrorInvalidParameter);
                return;
            }
            info->thing()->setStateValue(mockPushButtonColorStateTypeId, colorString);
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonPercentageActionTypeId) {
            info->thing()->setStateValue(mockPushButtonPercentageStateTypeId, info->action().param(mockPushButtonPercentageActionPercentageParamTypeId).value().toInt());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonAllowedValuesActionTypeId) {
            info->thing()->setStateValue(mockPushButtonAllowedValuesStateTypeId, info->action().param(mockPushButtonAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonDoubleActionTypeId) {
            info->thing()->setStateValue(mockPushButtonDoubleStateTypeId, info->action().param(mockPushButtonDoubleActionDoubleParamTypeId).value().toDouble());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonBoolActionTypeId) {
            info->thing()->setStateValue(mockPushButtonBoolStateTypeId, info->action().param(mockPushButtonBoolActionBoolParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockPushButtonTimeoutActionTypeId) {
            // Not finishing action intentionally...
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == mockDisplayPinThingClassId) {
        if (info->action().actionTypeId() == mockDisplayPinColorActionTypeId) {
            QString colorString = info->action().param(mockDisplayPinColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMockDevice) << "Invalid color parameter";
                info->finish(Thing::ThingErrorInvalidParameter);
                return;
            }
            info->thing()->setStateValue(mockDisplayPinColorStateTypeId, colorString);
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinPercentageActionTypeId) {
            info->thing()->setStateValue(mockDisplayPinPercentageStateTypeId, info->action().param(mockDisplayPinPercentageActionPercentageParamTypeId).value().toInt());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinAllowedValuesActionTypeId) {
            info->thing()->setStateValue(mockDisplayPinAllowedValuesStateTypeId, info->action().param(mockDisplayPinAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinDoubleActionTypeId) {
            info->thing()->setStateValue(mockDisplayPinDoubleStateTypeId, info->action().param(mockDisplayPinDoubleActionDoubleParamTypeId).value().toDouble());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinBoolActionTypeId) {
            info->thing()->setStateValue(mockDisplayPinBoolStateTypeId, info->action().param(mockDisplayPinBoolActionBoolParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == mockDisplayPinTimeoutActionTypeId) {
            // Not finishing action intentionally...
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == mockParentThingClassId) {
        if (info->action().actionTypeId() == mockParentBoolValueActionTypeId) {
            info->thing()->setStateValue(mockParentBoolValueStateTypeId, info->action().param(mockParentBoolValueActionBoolValueParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == mockChildThingClassId) {
        if (info->action().actionTypeId() == mockChildBoolValueActionTypeId) {
            info->thing()->setStateValue(mockChildBoolValueStateTypeId, info->action().param(mockChildBoolValueActionBoolValueParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == mockInputTypeThingClassId) {
        if (info->action().actionTypeId() == mockInputTypeWritableBoolActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableBoolStateTypeId, info->action().param(mockInputTypeWritableBoolActionWritableBoolParamTypeId).value().toULongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableIntActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableIntStateTypeId, info->action().param(mockInputTypeWritableIntActionWritableIntParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableIntMinMaxActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableIntMinMaxStateTypeId, info->action().param(mockInputTypeWritableIntMinMaxActionWritableIntMinMaxParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableUIntActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableUIntStateTypeId, info->action().param(mockInputTypeWritableUIntActionWritableUIntParamTypeId).value().toULongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableUIntMinMaxActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableUIntMinMaxStateTypeId, info->action().param(mockInputTypeWritableUIntMinMaxActionWritableUIntMinMaxParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableDoubleActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableDoubleStateTypeId, info->action().param(mockInputTypeWritableDoubleActionWritableDoubleParamTypeId).value().toDouble());
        } else if (info->action().actionTypeId() == mockInputTypeWritableDoubleMinMaxActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableDoubleMinMaxStateTypeId, info->action().param(mockInputTypeWritableDoubleMinMaxActionWritableDoubleMinMaxParamTypeId).value().toDouble());
        } else if (info->action().actionTypeId() == mockInputTypeWritableStringActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableStringStateTypeId, info->action().param(mockInputTypeWritableStringActionWritableStringParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == mockInputTypeWritableStringSelectionActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableStringSelectionStateTypeId, info->action().param(mockInputTypeWritableStringSelectionActionWritableStringSelectionParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == mockInputTypeWritableColorActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableColorStateTypeId, info->action().param(mockInputTypeWritableColorActionWritableColorParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == mockInputTypeWritableTimeActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableTimeStateTypeId, info->action().param(mockInputTypeWritableTimeActionWritableTimeParamTypeId).value().toTime());
        } else if (info->action().actionTypeId() == mockInputTypeWritableTimestampIntActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableTimestampIntStateTypeId, info->action().param(mockInputTypeWritableTimestampIntActionWritableTimestampIntParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == mockInputTypeWritableTimestampUIntActionTypeId) {
            info->thing()->setStateValue(mockInputTypeWritableTimestampUIntStateTypeId, info->action().param(mockInputTypeWritableTimestampUIntActionWritableTimestampUIntParamTypeId).value().toULongLong());
        }
        return;

    }
    info->finish(Thing::ThingErrorThingClassNotFound);
}

void DevicePluginMock::executeBrowserItem(BrowserActionInfo *info)
{
    qCDebug(dcMockDevice()) << "ExecuteBrowserItem called" << info->browserAction().itemId();
    bool broken = info->thing()->paramValue(mockDeviceBrokenParamTypeId).toBool();
    bool async = info->thing()->paramValue(mockDeviceAsyncParamTypeId).toBool();

    VirtualFsNode *node = m_virtualFs->findNode(info->browserAction().itemId());
    if (!node) {
        info->finish(Thing::ThingErrorItemNotFound);
        return;
    }

    if (!node->item.executable()) {
        info->finish(Thing::ThingErrorItemNotExecutable);
        return;
    }

    if (!async){
        if (broken) {
            info->finish(Thing::ThingErrorHardwareFailure);
            return;
        }
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    QTimer::singleShot(2000, info, [broken, info](){
        info->finish(broken ? Thing::ThingErrorHardwareFailure : Thing::ThingErrorNoError);
    });

}

void DevicePluginMock::executeBrowserItemAction(BrowserItemActionInfo *info)
{
    qCDebug(dcMockDevice()) << "TODO" << info << info->browserItemAction().id();
    if (info->browserItemAction().actionTypeId() == mockAddToFavoritesBrowserItemActionTypeId) {

        VirtualFsNode *node = m_virtualFs->findNode(info->browserItemAction().itemId());
        if (!node) {
            info->finish(Thing::ThingErrorInvalidParameter);
            return;
        }

        VirtualFsNode *favoritesNode = m_virtualFs->findNode("favorites");
        if (favoritesNode->findNode(info->browserItemAction().itemId())) {
            info->finish(Thing::ThingErrorThingInUse);
            return;
        }
        BrowserItem newItem = node->item;
        newItem.setActionTypeIds({mockRemoveFromFavoritesBrowserItemActionTypeId});
        VirtualFsNode *newNode = new VirtualFsNode(newItem);
        favoritesNode->addChild(newNode);
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->browserItemAction().actionTypeId() == mockRemoveFromFavoritesBrowserItemActionTypeId) {
        VirtualFsNode *favoritesNode = m_virtualFs->findNode("favorites");
        VirtualFsNode *nodeToRemove = favoritesNode->findNode(info->browserItemAction().itemId());
        if (!nodeToRemove) {
            info->finish(Thing::ThingErrorItemNotFound);
            return;
        }
        int idx = favoritesNode->childs.indexOf(nodeToRemove);
        delete favoritesNode->childs.takeAt(idx);
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    info->finish(Thing::ThingErrorActionTypeNotFound);
}

void DevicePluginMock::setState(const StateTypeId &stateTypeId, const QVariant &value)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Thing *device = m_daemons.key(daemon);
    device->setStateValue(stateTypeId, value);
}

void DevicePluginMock::triggerEvent(const EventTypeId &id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Thing *device = m_daemons.key(daemon);

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
    Thing *device = m_daemons.key(daemon);
    qCDebug(dcMockDevice) << "Emitting autoDeviceDisappeared for device" << device->id();
    emit autoThingDisappeared(device->id());
}

void DevicePluginMock::onReconfigureAutoDevice()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon *>(sender());
    if (!daemon)
        return;

    Thing *device = m_daemons.key(daemon);
    qCDebug(dcMockDevice()) << "Reconfigure auto device for" << device << device->params();

    int currentPort = device->params().paramValue(mockDeviceAutoDeviceHttpportParamTypeId).toInt();

    // Note: the reconfigure makes the http server listen on port + 1
    ParamList params;
    params.append(Param(mockDeviceAutoDeviceHttpportParamTypeId, currentPort + 1));

    ThingDescriptor deviceDescriptor(mockDeviceAutoThingClassId);
    deviceDescriptor.setTitle(device->name() + " (reconfigured)");
    deviceDescriptor.setDescription("This auto device was reconfigured");
    deviceDescriptor.setThingId(device->id());
    deviceDescriptor.setParams(params);

    emit autoThingsAppeared({deviceDescriptor});
}

void DevicePluginMock::generateDiscoveredDevices(ThingDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        ThingDescriptor d1(mockThingClassId, "Mock Device 1 (Discovered)", "55555");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55555");
        params.append(httpParam);
        d1.setParams(params);
        foreach (Thing *d, myThings()) {
            if (d->thingClassId() == mockThingClassId && d->paramValue(mockDeviceHttpportParamTypeId).toInt() == 55555) {
                d1.setThingId(d->id());
                break;
            }
        }
        info->addThingDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        ThingDescriptor d2(mockThingClassId, "Mock Device 2 (Discovered)", "55556");
        ParamList params;
        Param httpParam(mockDeviceHttpportParamTypeId, "55556");
        params.append(httpParam);
        d2.setParams(params);
        foreach (Thing *d, myThings()) {
            if (d->thingClassId() == mockThingClassId && d->paramValue(mockDeviceHttpportParamTypeId).toInt() == 55556) {
                d2.setThingId(d->id());
                break;
            }
        }
        info->addThingDescriptor(d2);
    }

    info->finish(Thing::ThingErrorNoError);
}

void DevicePluginMock::generateDiscoveredPushButtonDevices(ThingDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        ThingDescriptor d1(mockPushButtonThingClassId, "Mock Device (Push Button)", "1");
        info->addThingDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        ThingDescriptor d2(mockPushButtonThingClassId, "Mock Device (Push Button)", "2");
        info->addThingDescriptor(d2);
    }
    info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("This device will simulate a push button press in 3 seconds."));
}

void DevicePluginMock::generateDiscoveredDisplayPinDevices(ThingDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        ThingDescriptor d1(mockDisplayPinThingClassId, "Mock Device (Display Pin)", "1");
        foreach (Thing *existingDev, myThings()) {
            if (existingDev->thingClassId() == mockDisplayPinThingClassId) {
                d1.setThingId(existingDev->id());
                break;
            }
        }
        info->addThingDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        ThingDescriptor d2(mockDisplayPinThingClassId, "Mock Device (Display Pin)", "2");
        int count = 0;
        foreach (Thing *existingDev, myThings()) {
            if (existingDev->thingClassId() == mockDisplayPinThingClassId && ++count > 1) {
                d2.setThingId(existingDev->id());
                break;
            }
        }
        info->addThingDescriptor(d2);
    }

    info->finish(Thing::ThingErrorNoError);
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

    item = BrowserItem("mediaservices", "Media services", true, false);
    item.setDescription("I list media icons");
    item.setIcon(BrowserItem::BrowserIconMusic);
    VirtualFsNode *mediaNode = new VirtualFsNode(item);
    m_virtualFs->addChild(mediaNode);

    MediaBrowserItem mediaItem = MediaBrowserItem("playlist", "Playlists", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconPlaylist);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("recent", "Recently played", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconRecentlyPlayed);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("library", "Library", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconLibrary);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("musiclibrary", "Music Library", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconMusicLibrary);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("videolibrary", "Video library", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconVideoLibrary);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("picturelibrary", "picture library", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconPictureLibrary);
    mediaNode->addChild(new VirtualFsNode(mediaItem));


    mediaItem = MediaBrowserItem("disk", "CD", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconDisk);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("usb", "USB", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconUSB);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("network", "Network", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconNetwork);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("aux", "AUX", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconAux);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("spotify", "Spotify", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconSpotify);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("amazon", "Amazon Music", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconAmazon);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("tunein", "TuneIn", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconTuneIn);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("siriusxm", "Sirius XM", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconSiriusXM);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("vTuner", "vTuner", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconVTuner);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("tidal", "Tidal", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconTidal);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("airable", "airable", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconAirable);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("deezer", "Deezer", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconDeezer);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("napster", "Napster", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconNapster);
    mediaNode->addChild(new VirtualFsNode(mediaItem));

    mediaItem = MediaBrowserItem("soundcloud", "SoundCloud", false, false);
    mediaItem.setMediaIcon(MediaBrowserItem::MediaBrowserIconSoundCloud);
    mediaNode->addChild(new VirtualFsNode(mediaItem));
}
