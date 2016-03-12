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

#ifndef DEVICEPLUGINAWATTAR_H
#define DEVICEPLUGINAWATTAR_H

#include "plugin/deviceplugin.h"
#include "heatpump.h"

#include <QHash>
#include <QDebug>
#include <QTimer>
#include <QPointer>

class DevicePluginAwattar : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginawattar.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginAwattar();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void postSetupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

    void guhTimer() override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    QPointer<Device> m_device;
    QList<QPointer<HeatPump> > m_heatPumps;

    QList<QPointer<QNetworkReply> > m_searchPumpReplies;
    QList<QPointer<QNetworkReply> > m_updatePrice;
    QList<QPointer<QNetworkReply> > m_updateUserData;

    QString m_token;
    QString m_userUuid;

    int m_autoSgMode;
    int m_manualSgMode;


    QNetworkReply *requestPriceData(const QString& token);
    QNetworkReply *requestUserData(const QString& token, const QString &userId);

    void updateData();
    void searchHeatPumps();

    void processPriceData(const QVariantMap &data);
    void processUserData(const QVariantMap &data);
    void processPumpSearchData(const QByteArray &data);

    void setSgMode(const int &sgMode);
    void setOnlineStatus(const bool &online);
    bool heatPumpExists(const QHostAddress &pumpAddress);

private slots:
    void onHeatPumpReachableChanged();

};

#endif // DEVICEPLUGINAWATTAR_H
