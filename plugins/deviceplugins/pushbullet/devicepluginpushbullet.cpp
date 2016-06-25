/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Alexander Lampret <alexander.lampret@gmail.com>     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "devicepluginpushbullet.h"
#include "plugin/device.h"
#include "plugininfo.h"

DevicePluginPushbullet::DevicePluginPushbullet() {
}

DeviceManager::HardwareResources DevicePluginPushbullet::requiredHardware() const {
    return DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceSetupStatus DevicePluginPushbullet::setupDevice(Device *device) {
    Q_UNUSED(device);
    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginPushbullet::networkManagerReplyReady(QNetworkReply *reply)
{
    if (reply->error()) {
        qCWarning(dcPushbullet) << "Pushbullet reply error: " << reply->errorString();
        reply->deleteLater();
        return;
    }
    reply->deleteLater();
}

DeviceManager::DeviceError DevicePluginPushbullet::executeAction(Device *device, const Action &action) {
    if (device->deviceClassId() == pushNotificationDeviceClassId) {
        if (action.actionTypeId() == notifyActionTypeId) {
            sendNotification(device, action.params());
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginPushbullet::sendNotification(Device* device, ParamList params) {
    QUrlQuery urlParams;
    urlParams.addQueryItem("body", params.paramValue("body").toByteArray());
    urlParams.addQueryItem("title", params.paramValue("title").toByteArray());
    urlParams.addQueryItem("type", "note");
    QNetworkRequest request(QUrl("https://api.pushbullet.com/v2/pushes"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader(QByteArray("Access-Token"), device->paramValue("accessToken").toByteArray());
    networkManagerPost(request, urlParams.toString(QUrl::FullyEncoded).toUtf8());
}
