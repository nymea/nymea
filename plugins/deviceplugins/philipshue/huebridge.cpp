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

#include "huebridge.h"

HueBridge::HueBridge(QObject *parent) :
    QObject(parent),
    m_zigbeeChannel(-1)
{

}

QString HueBridge::name() const
{
    return m_name;
}

void HueBridge::setName(const QString &name)
{
    m_name = name;
}

QString HueBridge::id() const
{
    return m_id;
}

void HueBridge::setId(const QString &id)
{
    m_id = id;
}

QString HueBridge::apiKey() const
{
    return m_apiKey;
}

void HueBridge::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

QHostAddress HueBridge::hostAddress() const
{
    return m_hostAddress;
}

void HueBridge::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

QString HueBridge::macAddress() const
{
    return m_macAddress;
}

void HueBridge::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
}

QString HueBridge::apiVersion() const
{
    return m_apiVersion;
}

void HueBridge::setApiVersion(const QString &apiVersion)
{
    m_apiVersion = apiVersion;
}

QString HueBridge::softwareVersion() const
{
    return m_softwareVersion;
}

void HueBridge::setSoftwareVersion(const QString &softwareVersion)
{
    m_softwareVersion = softwareVersion;
}

int HueBridge::zigbeeChannel() const
{
    return m_zigbeeChannel;
}

void HueBridge::setZigbeeChannel(const int &zigbeeChannel)
{
    m_zigbeeChannel = zigbeeChannel;
}

QList<HueLight *> HueBridge::lights() const
{
    return m_lights;
}

void HueBridge::addLight(HueLight *light)
{
    m_lights.append(light);
}

QPair<QNetworkRequest, QByteArray> HueBridge::createDiscoverLightsRequest()
{
    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() + "/lights/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, QByteArray());
}

QPair<QNetworkRequest, QByteArray> HueBridge::createSearchLightsRequest()
{
    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() + "/lights/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, QByteArray());
}

QPair<QNetworkRequest, QByteArray> HueBridge::createSearchSensorsRequest()
{
    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() + "/sensors/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, QByteArray());
}

QPair<QNetworkRequest, QByteArray> HueBridge::createCheckUpdatesRequest()
{
    QVariantMap updateMap;
    updateMap.insert("checkforupdate", true);

    QVariantMap requestMap;
    requestMap.insert("portalservices", true);
    requestMap.insert("swupdate", updateMap);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() + "/config"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueBridge::createUpgradeRequest()
{
    QVariantMap updateMap;
    updateMap.insert("updatestate", 3);

    QVariantMap requestMap;
    requestMap.insert("swupdate", updateMap);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() + "/config"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}
