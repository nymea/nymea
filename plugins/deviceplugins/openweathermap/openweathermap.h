/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef OPENWEATHERMAP_H
#define OPENWEATHERMAP_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUrl>

class OpenWeatherMap : public QObject
{
    Q_OBJECT
public:
    explicit OpenWeatherMap(QObject *parent = 0);
    void update(QString id);
    void searchAutodetect();
    void search(QString searchString);

private:
    QNetworkAccessManager *m_manager;

    QString m_cityName;
    QString m_cityId;

    QNetworkReply *m_locationReply;
    QNetworkReply *m_searchLocationReply;
    QNetworkReply *m_weatherReply;
    QNetworkReply *m_searchReply;

    void processLocationResponse(QByteArray data);
    void processSearchResponse(QByteArray data);
    void processSearchLocationResponse(QByteArray data);

signals:
    void searchResultReady(const QList<QVariantMap> &cityList);
    void weatherDataReady(const QByteArray &data);

public slots:


private slots:
    void replyFinished(QNetworkReply *reply);

};

#endif // OPENWEATHERMAP_H
