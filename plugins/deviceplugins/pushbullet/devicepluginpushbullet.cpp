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
	return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginPushbullet::setupDevice(Device *device) {
    if (device->deviceClassId() == pushNotificationDeviceClassId) {
        // Start network manager for notifications
        m_manager = new QNetworkAccessManager(this);
        connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    }
	return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginPushbullet::deviceRemoved(Device *device) {
    if (device->deviceClassId() == pushNotificationDeviceClassId) {
        // Close manager
        m_manager->deleteLater();
    }
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
    QByteArray data = "{\"body\":\"" + params.paramValue("body").toByteArray() + "\",\"title\":\"" + params.paramValue("title").toByteArray() + "\",\"type\":\"note\"}";
    qCDebug(dcPushbullet) << "request will be sent with params " << data;
    QNetworkRequest request(QUrl("https://api.pushbullet.com/v2/pushes"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("Access-Token"), device->paramValue("accessToken").toByteArray());
    m_manager->post(request, data);
}

void DevicePluginPushbullet::replyFinished(QNetworkReply *reply) {
	QByteArray bytes = reply->readAll(); // bytes
    qCDebug(dcPushbullet) << "reply received" << bytes;
}
