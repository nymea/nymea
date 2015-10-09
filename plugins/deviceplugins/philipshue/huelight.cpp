/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include "huelight.h"
#include "extern-plugininfo.h"

HueLight::HueLight(const int &lightId, const QHostAddress &hostAddress, const QString &name, const QString &apiKey, const QString &modelId, const DeviceId &bridgeId, QObject *parent) :
    QObject(parent),
    m_lightId(lightId),
    m_hostAddress(hostAddress),
    m_name(name),
    m_apiKey(apiKey),
    m_modelId(modelId),
    m_bridgeId(bridgeId)
{
}

int HueLight::lightId() const
{
    return m_lightId;
}

void HueLight::setLightId(const int &lightId)
{
    m_lightId = lightId;
}

DeviceId HueLight::bridgeId() const
{
    return m_bridgeId;
}

void HueLight::setBridgeId(const DeviceId &bridgeDeviceId)
{
    m_bridgeId = bridgeDeviceId;
}

QHostAddress HueLight::hostAddress() const
{
    return m_hostAddress;
}

void HueLight::setHostAddress(const QHostAddress hostAddress)
{
    m_hostAddress = hostAddress;
}

QString HueLight::name() const
{
    return m_name;
}

void HueLight::setName(const QString &name)
{
    m_name = name;
}

QString HueLight::apiKey() const
{
    return m_apiKey;
}

void HueLight::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

QString HueLight::modelId() const
{
    return m_modelId;
}

void HueLight::setModelId(const QString &modelId)
{
    m_modelId = modelId;
}

QString HueLight::type() const
{
    return m_type;
}

void HueLight::setType(const QString &type)
{
    m_type = type;
}

QString HueLight::softwareVersion() const
{
    return m_softwareVersion;
}

void HueLight::setSoftwareVersion(const QString &softwareVersion)
{
    m_softwareVersion = softwareVersion;
}

bool HueLight::power() const
{
    return m_power;
}

void HueLight::setPower(const bool &power)
{
    m_power = power;
}

bool HueLight::reachable() const
{
    return m_reachable;
}

void HueLight::setReachable(const bool &reachable)
{
    m_reachable = reachable;
}

quint8 HueLight::brightness() const
{
    return m_brightness;
}

void HueLight::setBrigtness(const quint8 brightness)
{
    m_brightness = brightness;
}

quint16 HueLight::hue() const
{
    return m_hue;
}

void HueLight::setHue(const quint16 hue)
{
    m_hue = hue;
}

quint8 HueLight::sat() const
{
    return m_sat;
}

void HueLight::setSat(const quint8 sat)
{
    m_sat = sat;
}

QColor HueLight::color() const
{
    return QColor::fromHsv(m_hue * 360 / 65535, m_sat, 255);
}

QPointF HueLight::xy() const
{
    return m_xy;
}

void HueLight::setXy(const QPointF &xy)
{
    m_xy = xy;
}

quint16 HueLight::ct() const
{
    return m_ct;
}

void HueLight::setCt(const quint16 &ct)
{
    m_ct = ct;
}

QString HueLight::alert() const
{
    return m_alert;
}

void HueLight::setAlert(const QString &alert)
{
    m_alert = alert;
}

QString HueLight::effect() const
{
    return m_effect;
}

void HueLight::setEffect(const QString &effect)
{
    m_effect = effect;
}

HueLight::ColorMode HueLight::colorMode() const
{
    return m_colorMode;
}

void HueLight::setColorMode(const HueLight::ColorMode &colorMode)
{
    m_colorMode = colorMode;
}

void HueLight::updateStates(const QVariantMap &statesMap)
{
    // color mode
    if (statesMap.value("colormode").toString() == "hs") {
        setColorMode(ColorModeHS);
    } else if (statesMap.value("colormode").toString() == "ct") {
        setColorMode(ColorModeCT);
    } else if (statesMap.value("colormode").toString() == "xy") {
        setColorMode(ColorModeXY);
    }

    // effect (none, colorloop)
    if (statesMap.value("effect").toString() == "none") {
        setEffect("none");
    } else if (statesMap.value("effect").toString() == "colorloop") {
        setEffect("color loop");
    }

    // alert (none, select, lselect)
    setAlert(statesMap.value("alert").toString());
    setBrigtness(statesMap.value("bri").toInt());
    setCt(statesMap.value("ct").toInt());
    setPower(statesMap.value("on").toBool());
    setReachable(statesMap.value("reachable").toBool());
    setSat(statesMap.value("sat").toInt());
    setHue(statesMap.value("hue").toInt());
    //setXy(QPointF(statesMap.value("xy").toList().first().toFloat(),statesMap.value("xy").toList().last().toFloat()));

    emit stateChanged();
}

void HueLight::processActionResponse(const QVariantList &responseList)
{
    foreach (const QVariant &resultVariant, responseList) {
        QVariantMap result = resultVariant.toMap();
        if (result.contains("success")) {
            QVariantMap successMap = result.value("success").toMap();
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/on")) {
                m_power = successMap.value("/lights/" + QString::number(m_lightId) + "/state/on").toBool();
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/hue")) {
                m_hue = successMap.value("/lights/" + QString::number(m_lightId) + "/state/hue").toInt();
                m_colorMode = ColorModeHS;
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/bri")) {
                m_brightness = successMap.value("/lights/" + QString::number(m_lightId) + "/state/bri").toInt();
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/sat")) {
                m_sat = successMap.value("/lights/" + QString::number(m_lightId) + "/state/sat").toInt();
                m_colorMode = ColorModeHS;
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/xy")) {
                m_xy = successMap.value("/lights/" + QString::number(m_lightId) + "/state/xy").toPoint();
                m_colorMode = ColorModeXY;
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/ct")) {
                m_ct = successMap.value("/lights/" + QString::number(m_lightId) + "/state/ct").toInt();
                m_colorMode = ColorModeCT;
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/effect")) {
                QString effect = successMap.value("/lights/" + QString::number(m_lightId) + "/state/effect").toString();
                if (effect == "none") {
                    setEffect("none");
                } else if (effect == "colorloop") {
                    setEffect("color loop");
                }
            }
            if (successMap.contains("/lights/" + QString::number(m_lightId) + "/state/alert")) {
                m_alert = successMap.value("/lights/" + QString::number(m_lightId) + "/state/alert").toString();
            }

        }
    }
    emit stateChanged();
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetPowerRequest(const bool &power)
{
    QVariantMap requestMap;
    requestMap.insert("on", power);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(lightId()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetColorRequest(const QColor &color)
{
    QVariantMap requestMap;
    requestMap.insert("hue", color.hue() * 65535 / 360);
    requestMap.insert("sat", color.saturation());
    requestMap.insert("on", true);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(lightId()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetBrightnessRequest(const int &brightness)
{
    QVariantMap requestMap;
    requestMap.insert("bri", brightness);
    if (brightness == 0) {
        requestMap.insert("on", false);
    } else {
        requestMap.insert("on", true);
    }

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(lightId()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetEffectRequest(const QString &effect)
{
    QVariantMap requestMap;
    if (effect == "none") {
        requestMap.insert("effect", "none");
    } else if (effect == "color loop") {
        requestMap.insert("effect", "colorloop");
        requestMap.insert("on", true);
    }
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(lightId()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetTemperatureRequest(const int &colorTemp)
{
    QVariantMap requestMap;
    requestMap.insert("ct", colorTemp);
    requestMap.insert("on", true);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(lightId()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createFlashRequest(const QString &alert)
{
    QVariantMap requestMap;
    if (alert == "flash once") {
        requestMap.insert("alert", "select");
    } else if (alert == "flash 30 seconds") {
        requestMap.insert("alert", "lselect");
    }
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(lightId()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}
