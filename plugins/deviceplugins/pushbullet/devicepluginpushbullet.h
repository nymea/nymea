/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Alexander Lampret <alexander.lampret@gmail.com>     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPLUGINPUSHBULLET_H
#define DEVICEPLUGINPUSHBULLET_H

#include "plugin/deviceplugin.h"
#include "devicemanager.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QProcess>
#include <QUrlQuery>

class DevicePluginPushbullet: public DevicePlugin {
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginpushbullet.json")
	Q_INTERFACES(DevicePlugin)

public:
    DevicePluginPushbullet();
    DeviceManager::HardwareResources requiredHardware() const override;
	DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

public slots:
	DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    void sendNotification(Device* device, ParamList params);
};

#endif // DEVICEPLUGINPUSHBULLET_H
