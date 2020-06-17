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

#include "integrationpluginmock.h"
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

IntegrationPluginMock::IntegrationPluginMock()
{
    generateBrowseItems();
}

IntegrationPluginMock::~IntegrationPluginMock()
{
    delete m_virtualFs;
}

void IntegrationPluginMock::discoverThings(ThingDiscoveryInfo *info)
{
    if (info->thingClassId() == mockThingClassId) {
        qCDebug(dcMock()) << "starting mock discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(mockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info](){
            generateDiscoveredDevices(info);
        });
        return;
    }

    if (info->thingClassId() == pushButtonMockThingClassId) {
        qCDebug(dcMock()) << "starting mock push button discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(pushButtonMockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info]() {
            generateDiscoveredPushButtonDevices(info);
        });
        return;
    }

    if (info->thingClassId() == displayPinMockThingClassId) {
        qCDebug(dcMock()) << "starting mock display pin discovery:" << info->params();
        m_discoveredDeviceCount = info->params().paramValue(displayPinMockDiscoveryResultCountParamTypeId).toInt();
        QTimer::singleShot(1000, info, [this, info]() {
            generateDiscoveredDisplayPinDevices(info);
        });
        return;
    }

    if (info->thingClassId() == parentMockThingClassId) {
        qCDebug(dcMock()) << "Starting discovery for mocked parent thing";
        QTimer::singleShot(1000, info, [info](){
            ThingDescriptor descriptor(parentMockThingClassId, "Mocked Thing Parent (Discovered)");
            info->addThingDescriptor(descriptor);
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == childMockThingClassId) {
        QTimer::singleShot(1000, info, [this, info](){
            if (!myThings().filterByThingClassId(parentMockThingClassId).isEmpty()) {
                Thing *parent = myThings().filterByThingClassId(parentMockThingClassId).first();
                ThingDescriptor descriptor(childMockThingClassId, "Mocked Thing Child (Discovered)", QString(), parent->id());
                info->addThingDescriptor(descriptor);
            }
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == userAndPassMockThingClassId) {
        QTimer::singleShot(1000, info, [this, info](){
            if (myThings().filterByThingClassId(userAndPassMockThingClassId).isEmpty()) {
                ThingDescriptor descriptor(userAndPassMockThingClassId, "Mocked Thing User & Password (Discovered)", QString());
                info->addThingDescriptor(descriptor);
            }
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    qCWarning(dcMock()) << "Cannot discover for ThingClassId" << info->thingClassId();
    info->finish(Thing::ThingErrorThingNotFound);
}

void IntegrationPluginMock::setupThing(ThingSetupInfo *info)
{
    if (info->thing()->thingClassId() == mockThingClassId || info->thing()->thingClassId() == autoMockThingClassId) {
        if (m_daemons.contains(info->thing())) {
            // We already have a daemon, seem's we're reconfiguring
            delete m_daemons.take(info->thing());
        }

        bool async = false;
        bool broken = false;
        if (info->thing()->thingClassId() == mockThingClassId) {
            async = info->thing()->paramValue(mockThingAsyncParamTypeId).toBool();
            broken = info->thing()->paramValue(mockThingBrokenParamTypeId).toBool();
        } else {
            async = info->thing()->paramValue(autoMockThingAsyncParamTypeId).toBool();
            broken = info->thing()->paramValue(autoMockThingBrokenParamTypeId).toBool();
        }
        qCDebug(dcMock()) << "SetupThing for" << info->thing()->name() << "Async:" << async << "Broken:" << broken;

        if (!async && broken) {
            qCWarning(dcMock()) << "This thing is intentionally broken.";
            info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mocked thing is intentionally broken."));
            return;
        }

        if (!broken) {
            HttpDaemon *daemon = new HttpDaemon(info->thing(), this);
            m_daemons.insert(info->thing(), daemon);

            if (!daemon->isListening()) {
                qCWarning(dcMock()) << "HTTP port opening failed:" << info->thing()->paramValue(mockThingHttpportParamTypeId).toInt();
                info->finish(Thing::ThingErrorHardwareNotAvailable, QT_TR_NOOP("Failed to open HTTP port. Port in use?"));
                return;
            }

            connect(daemon, &HttpDaemon::triggerEvent, this, &IntegrationPluginMock::triggerEvent);
            connect(daemon, &HttpDaemon::setState, this, &IntegrationPluginMock::setState);
            // Keep this queued or it might happen that the HttpDaemon is deleted before it is able to reply to the caller
            connect(daemon, &HttpDaemon::disappear, this, &IntegrationPluginMock::onDisappear, Qt::QueuedConnection);
            connect(daemon, &HttpDaemon::reconfigureAutodevice, this, &IntegrationPluginMock::onReconfigureAutoDevice, Qt::QueuedConnection);
        }


        if (async) {
            Thing *device = info->thing();
            QTimer::singleShot(1000, device, [info](){
                qCDebug(dcMock()) << "Finishing thing setup for mocked thing" << info->thing()->name();
                if (info->thing()->paramValue(mockThingBrokenParamTypeId).toBool()) {
                    info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mocked thing is intentionally broken."));
                } else {
                    info->finish(Thing::ThingErrorNoError);
                }
            });
            return;
        }
        qCDebug(dcMock()) << "Setup complete" << info->thing()->name();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == pushButtonMockThingClassId) {
        qCDebug(dcMock()) << "Setup PushButton mock thing" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == displayPinMockThingClassId) {
        qCDebug(dcMock()) << "Setup DisplayPin mock thing" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == parentMockThingClassId) {
        qCDebug(dcMock()) << "Setup Parent mock thing" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == childMockThingClassId) {
        qCDebug(dcMock()) << "Setup Child mock thing" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == inputTypeMockThingClassId) {
        qCDebug(dcMock()) << "Setup InputType mock thing" << info->thing()->params();
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == userAndPassMockThingClassId) {
        qCDebug(dcMock()) << "Setup User and password mock thing";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == oAuthGoogleMockThingClassId) {
        qCDebug(dcMock()) << "Google OAuth setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == oAuthSonosMockThingClassId) {
        qCDebug(dcMock()) << "Sonos OAuth setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == genericIoMockThingClassId) {
        qCDebug(dcMock()) << "Generic IO mock setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == virtualIoLightMockThingClassId) {
        qCDebug(dcMock()) << "Virtual IO mock light setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == virtualIoTemperatureSensorMockThingClassId) {
        qCDebug(dcMock()) << "Virtual IO mock temperature sensor setup complete";
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    qCWarning(dcMock()) << "Unhandled thing class" << info->thing()->thingClass();
    info->finish(Thing::ThingErrorThingClassNotFound);
}

void IntegrationPluginMock::postSetupThing(Thing *device)
{
    qCDebug(dcMock()) << "Postsetup mock" << device->name();
    if (device->thingClassId() == parentMockThingClassId) {
        foreach (Thing *d, myThings()) {
            if (d->thingClassId() == childMockThingClassId && d->parentId() == device->id()) {
                return;
            }
        }

        ThingDescriptor mockDescriptor(childMockThingClassId, "Mocked Thing Child (Auto created)", "Mocked Thing Child (Auto created)", device->id());
        emit autoThingsAppeared(ThingDescriptors() << mockDescriptor);
    }
}

void IntegrationPluginMock::thingRemoved(Thing *device)
{
    delete m_daemons.take(device);
}

void IntegrationPluginMock::startMonitoringAutoThings()
{
    foreach (Thing *device, myThings()) {
        if (device->thingClassId() == autoMockThingClassId) {
            return; // We already have a Auto Mock device... do nothing.
        }
    }

    ThingDescriptor mockDescriptor(autoMockThingClassId, "Mocked Thing (Auto created)");

    ParamList params;
    qsrand(QDateTime::currentMSecsSinceEpoch());
    int port = 4242 + (qrand() % 1000);
    Param param(autoMockThingHttpportParamTypeId, port);
    params.append(param);
    mockDescriptor.setParams(params);

    QList<ThingDescriptor> deviceDescriptorList;
    deviceDescriptorList.append(mockDescriptor);

    emit autoThingsAppeared(deviceDescriptorList);
}

void IntegrationPluginMock::startPairing(ThingPairingInfo *info)
{
    if (info->thingClassId() == pushButtonMockThingClassId) {
        qCDebug(dcMock()) << "Push button. Pressing the button in 3 seconds.";
        info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("Wait 3 second before you continue, the push button will be pressed automatically."));
        m_pushbuttonPressed = false;
        QTimer::singleShot(3000, this, SLOT(onPushButtonPressed()));
        return;
    }

    if (info->thingClassId() == displayPinMockThingClassId) {
        qCDebug(dcMock()) << "Display pin!! The pin is 243681";
        info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("Please enter the secret which normaly will be displayed on the device. For this mocked thing the pin is 243681."));
        return;
    }

    if (info->thingClassId() == userAndPassMockThingClassId) {
        qCDebug(dcMock()) << "User and password. Login is \"user\" and \"password\".";
        info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("Please enter login credentials for the mocked thing (\"user\" and \"password\")."));
        return;
    }

    if (info->thingClassId() == oAuthSonosMockThingClassId) {
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

        qCDebug(dcMock()) << "Sonos url:" << url;

        info->setOAuthUrl(url);
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thingClassId() == oAuthGoogleMockThingClassId) {
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

void IntegrationPluginMock::confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret)
{
    qCDebug(dcMock()) << "Confirm pairing";

    if (info->thingClassId() == pushButtonMockThingClassId) {
        if (!m_pushbuttonPressed) {
            qCDebug(dcMock()) << "PushButton not pressed yet!";
            info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("The push button has not been pressed."));
            return;
        }

        QTimer::singleShot(1000, this, [info](){
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == displayPinMockThingClassId) {
        if (secret != "243681") {
            qCWarning(dcMock()) << "Invalid pin:" << secret;
            info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("Invalid PIN!"));
            return;
        }
        QTimer::singleShot(500, this, [info](){
            qCDebug(dcMock()) << "Pairing finished.";
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }

    if (info->thingClassId() == userAndPassMockThingClassId) {
        qCDebug(dcMock()) << "Credentials received:" << username << secret;
        if (username == "user" && secret == "password") {
            info->finish(Thing::ThingErrorNoError);
            return;
        } else {
            info->finish(Thing::ThingErrorAuthenticationFailure, QT_TR_NOOP("Wrong username or password"));
            return;
        }
    }


    if (info->thingClassId() == oAuthSonosMockThingClassId) {
        qCDebug(dcMock()) << "Secret is" << secret;
        QUrl url(secret);
        QUrlQuery query(url);
        qCDebug(dcMock()) << "Acess code is:" << query.queryItemValue("code");

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
            qCDebug(dcMock()) << "Sonos accessToken reply:" << this << reply->error() << reply->errorString() << jsonDoc.toJson();
            qCDebug(dcMock()) << "Access token:" << jsonDoc.toVariant().toMap().value("access_token").toString();
            qCDebug(dcMock()) << "expires at" << QDateTime::currentDateTime().addSecs(jsonDoc.toVariant().toMap().value("expires_in").toInt()).toString();
            qCDebug(dcMock()) << "Refresh token:" << jsonDoc.toVariant().toMap().value("refresh_token").toString();
            info->finish(Thing::ThingErrorNoError);
        });

        return;
    }


    if (info->thingClassId() == oAuthGoogleMockThingClassId) {
        qCDebug(dcMock()) << "Secret is" << secret;
        QUrl url(secret);
        QUrlQuery query(url);
        qCDebug(dcMock()) << "Acess code is:" << query.queryItemValue("code");

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
            qCDebug(dcMock()) << "Sonos accessToken reply:" << this << reply->error() << reply->errorString() << jsonDoc.toJson();
            qCDebug(dcMock()) << "Access token:" << jsonDoc.toVariant().toMap().value("access_token").toString();
            qCDebug(dcMock()) << "expires at" << QDateTime::currentDateTime().addSecs(jsonDoc.toVariant().toMap().value("expires_in").toInt()).toString();
            qCDebug(dcMock()) << "Refresh token:" << jsonDoc.toVariant().toMap().value("refresh_token").toString();
            qCDebug(dcMock()) << "ID token:" << jsonDoc.toVariant().toMap().value("id_token").toString();
            info->finish(Thing::ThingErrorNoError);
        });
        return;
    }


    qCWarning(dcMock()) << "Invalid ThingClassId -> no pairing possible with this thing";
    info->finish(Thing::ThingErrorThingClassNotFound);
}

void IntegrationPluginMock::browseThing(BrowseResult *result)
{
    qCDebug(dcMock()) << "Browse thing called" << result->thing();
    if (result->thing()->thingClassId() == mockThingClassId) {
        if (result->thing()->paramValue(mockThingAsyncParamTypeId).toBool()) {

            QTimer::singleShot(1000, result, [this, result]() {
                if (result->thing()->paramValue(mockThingBrokenParamTypeId).toBool()) {
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

        if (result->thing()->paramValue(mockThingBrokenParamTypeId).toBool()) {
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

void IntegrationPluginMock::browserItem(BrowserItemResult *result)
{
    VirtualFsNode *node = m_virtualFs->findNode(result->itemId());
    if (!node) {
        result->finish(Thing::ThingErrorItemNotFound);
        return;
    }
    result->finish(node->item);
}

void IntegrationPluginMock::executeAction(ThingActionInfo *info)
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
            qCDebug(dcMock()) << "Setting power to" << info->action().param(mockPowerActionPowerParamTypeId).value().toBool();
            info->thing()->setStateValue(mockPowerStateTypeId, info->action().param(mockPowerActionPowerParamTypeId).value().toBool());
        }
        m_daemons.value(info->thing())->actionExecuted(info->action().actionTypeId());
        info->finish(Thing::ThingErrorNoError);
        return;
    }

    if (info->thing()->thingClassId() == autoMockThingClassId) {
        if (info->action().actionTypeId() == autoMockMockActionAsyncActionTypeId || info->action().actionTypeId() == autoMockMockActionAsyncBrokenActionTypeId) {
            QTimer::singleShot(1000, info->thing(), [info](){
                if (info->action().actionTypeId() == autoMockMockActionAsyncBrokenActionTypeId) {
                    info->finish(Thing::ThingErrorSetupFailed, QT_TR_NOOP("This mock action is intentionally broken."));
                } else {
                    info->finish(Thing::ThingErrorNoError);
                }
            });
        }

        if (info->action().actionTypeId() == autoMockMockActionBrokenActionTypeId) {
            info->finish(Thing::ThingErrorSetupFailed);
            return;
        }

        m_daemons.value(info->thing())->actionExecuted(info->action().actionTypeId());
        info->finish(Thing::ThingErrorNoError);
    } else if (info->thing()->thingClassId() == pushButtonMockThingClassId) {
        if (info->action().actionTypeId() == pushButtonMockColorActionTypeId) {
            QString colorString = info->action().param(pushButtonMockColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMock()) << "Invalid color parameter";
                info->finish(Thing::ThingErrorInvalidParameter);
                return;
            }
            info->thing()->setStateValue(pushButtonMockColorStateTypeId, colorString);
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == pushButtonMockPercentageActionTypeId) {
            info->thing()->setStateValue(pushButtonMockPercentageStateTypeId, info->action().param(pushButtonMockPercentageActionPercentageParamTypeId).value().toInt());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == pushButtonMockAllowedValuesActionTypeId) {
            info->thing()->setStateValue(pushButtonMockAllowedValuesStateTypeId, info->action().param(pushButtonMockAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == pushButtonMockDoubleActionTypeId) {
            info->thing()->setStateValue(pushButtonMockDoubleStateTypeId, info->action().param(pushButtonMockDoubleActionDoubleParamTypeId).value().toDouble());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == pushButtonMockBoolActionTypeId) {
            info->thing()->setStateValue(pushButtonMockBoolStateTypeId, info->action().param(pushButtonMockBoolActionBoolParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == pushButtonMockTimeoutActionTypeId) {
            // Not finishing action intentionally...
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == displayPinMockThingClassId) {
        if (info->action().actionTypeId() == displayPinMockColorActionTypeId) {
            QString colorString = info->action().param(displayPinMockColorActionColorParamTypeId).value().toString();
            QColor color(colorString);
            if (!color.isValid()) {
                qCWarning(dcMock()) << "Invalid color parameter";
                info->finish(Thing::ThingErrorInvalidParameter);
                return;
            }
            info->thing()->setStateValue(displayPinMockColorStateTypeId, colorString);
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == displayPinMockPercentageActionTypeId) {
            info->thing()->setStateValue(displayPinMockPercentageStateTypeId, info->action().param(displayPinMockPercentageActionPercentageParamTypeId).value().toInt());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == displayPinMockAllowedValuesActionTypeId) {
            info->thing()->setStateValue(displayPinMockAllowedValuesStateTypeId, info->action().param(displayPinMockAllowedValuesActionAllowedValuesParamTypeId).value().toString());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == displayPinMockDoubleActionTypeId) {
            info->thing()->setStateValue(displayPinMockDoubleStateTypeId, info->action().param(displayPinMockDoubleActionDoubleParamTypeId).value().toDouble());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == displayPinMockBoolActionTypeId) {
            info->thing()->setStateValue(displayPinMockBoolStateTypeId, info->action().param(displayPinMockBoolActionBoolParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        } else if (info->action().actionTypeId() == displayPinMockTimeoutActionTypeId) {
            // Not finishing action intentionally...
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == parentMockThingClassId) {
        if (info->action().actionTypeId() == parentMockBoolValueActionTypeId) {
            info->thing()->setStateValue(parentMockBoolValueStateTypeId, info->action().param(parentMockBoolValueActionBoolValueParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == childMockThingClassId) {
        if (info->action().actionTypeId() == childMockBoolValueActionTypeId) {
            info->thing()->setStateValue(childMockBoolValueStateTypeId, info->action().param(childMockBoolValueActionBoolValueParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        info->finish(Thing::ThingErrorActionTypeNotFound);
        return;
    } else if (info->thing()->thingClassId() == inputTypeMockThingClassId) {
        if (info->action().actionTypeId() == inputTypeMockWritableBoolActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableBoolStateTypeId, info->action().param(inputTypeMockWritableBoolActionWritableBoolParamTypeId).value().toULongLong());
        } else if (info->action().actionTypeId() == inputTypeMockWritableIntActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableIntStateTypeId, info->action().param(inputTypeMockWritableIntActionWritableIntParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == inputTypeMockWritableIntMinMaxActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableIntMinMaxStateTypeId, info->action().param(inputTypeMockWritableIntMinMaxActionWritableIntMinMaxParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == inputTypeMockWritableUIntActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableUIntStateTypeId, info->action().param(inputTypeMockWritableUIntActionWritableUIntParamTypeId).value().toULongLong());
        } else if (info->action().actionTypeId() == inputTypeMockWritableUIntMinMaxActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableUIntMinMaxStateTypeId, info->action().param(inputTypeMockWritableUIntMinMaxActionWritableUIntMinMaxParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == inputTypeMockWritableDoubleActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableDoubleStateTypeId, info->action().param(inputTypeMockWritableDoubleActionWritableDoubleParamTypeId).value().toDouble());
        } else if (info->action().actionTypeId() == inputTypeMockWritableDoubleMinMaxActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableDoubleMinMaxStateTypeId, info->action().param(inputTypeMockWritableDoubleMinMaxActionWritableDoubleMinMaxParamTypeId).value().toDouble());
        } else if (info->action().actionTypeId() == inputTypeMockWritableStringActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableStringStateTypeId, info->action().param(inputTypeMockWritableStringActionWritableStringParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == inputTypeMockWritableStringSelectionActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableStringSelectionStateTypeId, info->action().param(inputTypeMockWritableStringSelectionActionWritableStringSelectionParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == inputTypeMockWritableColorActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableColorStateTypeId, info->action().param(inputTypeMockWritableColorActionWritableColorParamTypeId).value().toString());
        } else if (info->action().actionTypeId() == inputTypeMockWritableTimeActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableTimeStateTypeId, info->action().param(inputTypeMockWritableTimeActionWritableTimeParamTypeId).value().toTime());
        } else if (info->action().actionTypeId() == inputTypeMockWritableTimestampIntActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableTimestampIntStateTypeId, info->action().param(inputTypeMockWritableTimestampIntActionWritableTimestampIntParamTypeId).value().toLongLong());
        } else if (info->action().actionTypeId() == inputTypeMockWritableTimestampUIntActionTypeId) {
            info->thing()->setStateValue(inputTypeMockWritableTimestampUIntStateTypeId, info->action().param(inputTypeMockWritableTimestampUIntActionWritableTimestampUIntParamTypeId).value().toULongLong());
        }
        return;
    }

    if (info->thing()->thingClassId() == virtualIoLightMockThingClassId) {
        if (info->action().actionTypeId() == virtualIoLightMockPowerActionTypeId) {
            qCDebug(dcMock()) << "ExecuteAction for virtual light power action with param" << info->action().param(virtualIoLightMockPowerActionPowerParamTypeId).value();
            info->thing()->setStateValue(virtualIoLightMockPowerStateTypeId, info->action().param(virtualIoLightMockPowerActionPowerParamTypeId).value());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
    }

    if (info->thing()->thingClassId() == genericIoMockThingClassId) {
        if (info->action().actionTypeId() == genericIoMockDigitalOutput1ActionTypeId) {
            qCDebug(dcMock()) << "Setting digital output 1 to" << info->action().param(genericIoMockDigitalOutput1ActionDigitalOutput1ParamTypeId).value().toBool();
            info->thing()->setStateValue(genericIoMockDigitalOutput1StateTypeId, info->action().param(genericIoMockDigitalOutput1ActionDigitalOutput1ParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        if (info->action().actionTypeId() == genericIoMockDigitalOutput2ActionTypeId) {
            info->thing()->setStateValue(genericIoMockDigitalOutput2StateTypeId, info->action().param(genericIoMockDigitalOutput2ActionDigitalOutput2ParamTypeId).value().toBool());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        if (info->action().actionTypeId() == genericIoMockAnalogInput1ActionTypeId) {
            qCDebug(dcMock()) << "ExecuteAction for virtual io analog in 1 action with param" << info->action().param(genericIoMockAnalogInput1ActionAnalogInput1ParamTypeId).value();
            info->thing()->setStateValue(genericIoMockAnalogInput1StateTypeId, info->action().param(genericIoMockAnalogInput1ActionAnalogInput1ParamTypeId).value());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        if (info->action().actionTypeId() == genericIoMockAnalogOutput1ActionTypeId) {
            info->thing()->setStateValue(genericIoMockAnalogOutput1StateTypeId, info->action().param(genericIoMockAnalogOutput1ActionAnalogOutput1ParamTypeId).value());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
        if (info->action().actionTypeId() == genericIoMockAnalogOutput2ActionTypeId) {
            info->thing()->setStateValue(genericIoMockAnalogOutput2StateTypeId, info->action().param(genericIoMockAnalogOutput2ActionAnalogOutput2ParamTypeId).value());
            info->finish(Thing::ThingErrorNoError);
            return;
        }
    }

    if (info->thing()->thingClassId() == virtualIoTemperatureSensorMockThingClassId) {
        if (info->action().actionTypeId() == virtualIoTemperatureSensorMockInputActionTypeId) {
            double value = info->action().param(virtualIoTemperatureSensorMockInputActionInputParamTypeId).value().toDouble();
            info->thing()->setStateValue(virtualIoTemperatureSensorMockInputStateTypeId, value);
            double minTemp = info->thing()->setting(virtualIoTemperatureSensorMockSettingsMinTempParamTypeId).toDouble();
            double maxTemp = info->thing()->setting(virtualIoTemperatureSensorMockSettingsMaxTempParamTypeId).toDouble();
            double temp = minTemp + (maxTemp - minTemp) * value;
            info->thing()->setStateValue(virtualIoTemperatureSensorMockTemperatureStateTypeId, temp);
            info->finish(Thing::ThingErrorNoError);
            return;
        }
    }

    qCWarning(dcMock()) << "Unhandled executeAction call in mock plugin!";
}

void IntegrationPluginMock::executeBrowserItem(BrowserActionInfo *info)
{
    qCDebug(dcMock()) << "ExecuteBrowserItem called" << info->browserAction().itemId();
    bool broken = info->thing()->paramValue(mockThingBrokenParamTypeId).toBool();
    bool async = info->thing()->paramValue(mockThingAsyncParamTypeId).toBool();

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

void IntegrationPluginMock::executeBrowserItemAction(BrowserItemActionInfo *info)
{
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

void IntegrationPluginMock::setState(const StateTypeId &stateTypeId, const QVariant &value)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Thing *device = m_daemons.key(daemon);
    device->setStateValue(stateTypeId, value);
}

void IntegrationPluginMock::triggerEvent(const EventTypeId &id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon)
        return;

    Thing *device = m_daemons.key(daemon);

    qCDebug(dcMock) << "Emitting event " << id;
    device->emitEvent(id);
}

void IntegrationPluginMock::onDisappear()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }
    Thing *device = m_daemons.key(daemon);
    qCDebug(dcMock) << "Emitting autoDeviceDisappeared for device" << device->id();
    emit autoThingDisappeared(device->id());
}

void IntegrationPluginMock::onReconfigureAutoDevice()
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon *>(sender());
    if (!daemon)
        return;

    Thing *device = m_daemons.key(daemon);
    qCDebug(dcMock()) << "Reconfigure auto device for" << device << device->params();

    int currentPort = device->params().paramValue(autoMockThingHttpportParamTypeId).toInt();

    // Note: the reconfigure makes the http server listen on port + 1
    ParamList params;
    params.append(Param(autoMockThingHttpportParamTypeId, currentPort + 1));

    ThingDescriptor deviceDescriptor(autoMockThingClassId);
    deviceDescriptor.setTitle(device->name() + " (reconfigured)");
    deviceDescriptor.setDescription("This auto device was reconfigured");
    deviceDescriptor.setThingId(device->id());
    deviceDescriptor.setParams(params);

    emit autoThingsAppeared({deviceDescriptor});
}

void IntegrationPluginMock::generateDiscoveredDevices(ThingDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        ThingDescriptor d1(mockThingClassId, "Mock Device 1 (Discovered)", "55555");
        ParamList params;
        Param httpParam(mockThingHttpportParamTypeId, "55555");
        params.append(httpParam);
        d1.setParams(params);
        foreach (Thing *d, myThings()) {
            if (d->thingClassId() == mockThingClassId && d->paramValue(mockThingHttpportParamTypeId).toInt() == 55555) {
                d1.setThingId(d->id());
                break;
            }
        }
        info->addThingDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        ThingDescriptor d2(mockThingClassId, "Mock Device 2 (Discovered)", "55556");
        ParamList params;
        Param httpParam(mockThingHttpportParamTypeId, "55556");
        params.append(httpParam);
        d2.setParams(params);
        foreach (Thing *d, myThings()) {
            if (d->thingClassId() == mockThingClassId && d->paramValue(mockThingHttpportParamTypeId).toInt() == 55556) {
                d2.setThingId(d->id());
                break;
            }
        }
        info->addThingDescriptor(d2);
    }

    info->finish(Thing::ThingErrorNoError);
}

void IntegrationPluginMock::generateDiscoveredPushButtonDevices(ThingDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        ThingDescriptor d1(pushButtonMockThingClassId, "Mocked Thing (Push Button)", "1");
        info->addThingDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        ThingDescriptor d2(pushButtonMockThingClassId, "Mocked Thhing (Push Button)", "2");
        info->addThingDescriptor(d2);
    }
    info->finish(Thing::ThingErrorNoError, QT_TR_NOOP("This thing will simulate a push button press in 3 seconds."));
}

void IntegrationPluginMock::generateDiscoveredDisplayPinDevices(ThingDiscoveryInfo *info)
{
    if (m_discoveredDeviceCount > 0) {
        ThingDescriptor d1(displayPinMockThingClassId, "Mocked Thing (Display Pin)", "1");
        foreach (Thing *existingDev, myThings()) {
            if (existingDev->thingClassId() == displayPinMockThingClassId) {
                d1.setThingId(existingDev->id());
                break;
            }
        }
        info->addThingDescriptor(d1);
    }

    if (m_discoveredDeviceCount > 1) {
        ThingDescriptor d2(displayPinMockThingClassId, "Mocked Thing (Display Pin)", "2");
        int count = 0;
        foreach (Thing *existingDev, myThings()) {
            if (existingDev->thingClassId() == displayPinMockThingClassId && ++count > 1) {
                d2.setThingId(existingDev->id());
                break;
            }
        }
        info->addThingDescriptor(d2);
    }

    info->finish(Thing::ThingErrorNoError);
}

void IntegrationPluginMock::onPushButtonPressed()
{
    qCDebug(dcMock) << "PushButton pressed (automatically)";
    m_pushbuttonPressed = true;
}

void IntegrationPluginMock::onPluginConfigChanged()
{

}

void IntegrationPluginMock::generateBrowseItems()
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
