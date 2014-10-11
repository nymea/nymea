/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef OPENWEATHERMAP_H
#define OPENWEATHERMAP_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QVariantMap>
#include <QHostAddress>
#include <QUrl>

#include "plugin/deviceplugin.h"

class OpenWeatherMap : public QObject
{
    Q_OBJECT
public:
    explicit OpenWeatherMap(QObject *parent = 0);
    void update(QString id, DeviceId deviceId);
    void searchAutodetect();
    void search(QString searchString);
    void searchGeoLocation(double lat, double lon);

private:
    QNetworkAccessManager *m_manager;

    QString m_cityName;
    QString m_country;
    QString m_cityId;
    QHostAddress m_wanIp;

    QNetworkReply *m_locationReply;
    QNetworkReply *m_weatherReply;
    QNetworkReply *m_searchReply;
    QNetworkReply *m_searchGeoReply;

    void processLocationResponse(QByteArray data);
    void processSearchResponse(QByteArray data);
    void processSearchGeoResponse(QByteArray data);

    QHash<QNetworkReply*,DeviceId> m_weatherReplys;

signals:
    void searchResultReady(const QList<QVariantMap> &cityList);
    void weatherDataReady(const QByteArray &data, const DeviceId &deviceId);

public slots:


private slots:
    void replyFinished(QNetworkReply *reply);

};

#endif // OPENWEATHERMAP_H
