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
	void deviceRemoved(Device *device) override;

public slots:
	DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    QNetworkAccessManager *m_manager;
    void sendNotification(Device* device, ParamList params);

private slots:
	void replyFinished(QNetworkReply *reply);
};

#endif // DEVICEPLUGINPUSHBULLET_H
