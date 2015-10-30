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

#ifndef DEVICEPLUGINOPENWEATHERMAP_H
#define DEVICEPLUGINOPENWEATHERMAP_H

#include "plugin/deviceplugin.h"

#include <QTimer>

class DevicePluginOpenweathermap : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginopenweathermap.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginOpenweathermap();

    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;
    //void guhTimer() override;

private:
    QList<QNetworkReply *> m_autodetectionReplies;
    QList<QNetworkReply *> m_searchReplies;
    QList<QNetworkReply *> m_searchGeoReplies;
    QHash<QNetworkReply *, Device *> m_weatherReplies;

    // Autodetection data
    QHostAddress m_wanIp;
    QString m_country;
    QString m_cityName;
    double m_longitude;
    double m_latitude;

    QTimer *m_timer;
    QString m_apiKey;

    void update(Device *device);
    void searchAutodetect();
    void search(QString searchString);
    void searchGeoLocation(double lat, double lon);

    void processAutodetectResponse(QByteArray data);
    void processSearchResponse(QByteArray data);
    void processGeoSearchResponse(QByteArray data);

    void processSearchResults(const QList<QVariantMap> &cityList);
    void processWeatherData(const QByteArray &data, Device *device);

private slots:
    void onTimeout();

};

#endif // DEVICEPLUGINOPENWEATHERMAP_H
