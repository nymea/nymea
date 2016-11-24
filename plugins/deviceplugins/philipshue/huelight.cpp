/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

HueLight::HueLight(QObject *parent) :
    HueDevice(parent)
{
}

bool HueLight::power() const
{
    return m_power;
}

void HueLight::setPower(const bool &power)
{
    m_power = power;
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

    setReachable(statesMap.value("reachable").toBool());

    // alert (none, select, lselect)
    setAlert(statesMap.value("alert").toString());
    setBrigtness(statesMap.value("bri").toInt());
    setCt(statesMap.value("ct").toInt());
    setPower(statesMap.value("on").toBool());
    setSat(statesMap.value("sat").toInt());
    setHue(statesMap.value("hue").toInt());
    if (!statesMap.value("xy").toList().isEmpty())
        setXy(QPointF(statesMap.value("xy").toList().first().toFloat(), statesMap.value("xy").toList().last().toFloat()));

    emit stateChanged();
}

void HueLight::processActionResponse(const QVariantList &responseList)
{
    foreach (const QVariant &resultVariant, responseList) {
        QVariantMap result = resultVariant.toMap();
        if (result.contains("success")) {
            QVariantMap successMap = result.value("success").toMap();
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/on")) {
                m_power = successMap.value("/lights/" + QString::number(id()) + "/state/on").toBool();
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/hue")) {
                m_hue = successMap.value("/lights/" + QString::number(id()) + "/state/hue").toInt();
                m_colorMode = ColorModeHS;
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/bri")) {
                m_brightness = successMap.value("/lights/" + QString::number(id()) + "/state/bri").toInt();
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/sat")) {
                m_sat = successMap.value("/lights/" + QString::number(id()) + "/state/sat").toInt();
                m_colorMode = ColorModeHS;
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/xy")) {
                m_xy = successMap.value("/lights/" + QString::number(id()) + "/state/xy").toPoint();
                m_colorMode = ColorModeXY;
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/ct")) {
                m_ct = successMap.value("/lights/" + QString::number(id()) + "/state/ct").toInt();
                m_colorMode = ColorModeCT;
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/effect")) {
                QString effect = successMap.value("/lights/" + QString::number(id()) + "/state/effect").toString();
                if (effect == "none") {
                    setEffect("none");
                } else if (effect == "colorloop") {
                    setEffect("color loop");
                }
            }
            if (successMap.contains("/lights/" + QString::number(id()) + "/state/alert")) {
                m_alert = successMap.value("/lights/" + QString::number(id()) + "/state/alert").toString();
            }

        }
    }
    emit stateChanged();
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetPowerRequest(const bool &power)
{
    qCDebug(dcPhilipsHue()) << "Create power request" << power;

    QVariantMap requestMap;
    requestMap.insert("on", power);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(id()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetColorRequest(const QColor &color)
{
    qCDebug(dcPhilipsHue()) << "Create color request" << color.toRgb();

    QVariantMap requestMap;
    requestMap.insert("hue", color.hue() * 65535 / 360);
    requestMap.insert("sat", color.saturation());
    requestMap.insert("on", true);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(id()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetBrightnessRequest(const int &brightness)
{
    qCDebug(dcPhilipsHue()) << "Create brightness request" << brightness;

    QVariantMap requestMap;
    requestMap.insert("bri", brightness);
    if (brightness == 0) {
        requestMap.insert("on", false);
    } else {
        requestMap.insert("on", true);
    }

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(id()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetEffectRequest(const QString &effect)
{
    qCDebug(dcPhilipsHue()) << "Create effect request" << effect;

    QVariantMap requestMap;
    if (effect == "none") {
        requestMap.insert("effect", "none");
    } else if (effect == "color loop") {
        requestMap.insert("effect", "colorloop");
        requestMap.insert("on", true);
    }
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(id()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createSetTemperatureRequest(const int &colorTemp)
{
    qCDebug(dcPhilipsHue()) << "Create color temperature request" << colorTemp;

    QVariantMap requestMap;
    requestMap.insert("ct", colorTemp);
    requestMap.insert("on", true);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(id()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}

QPair<QNetworkRequest, QByteArray> HueLight::createFlashRequest(const QString &alert)
{
    qCDebug(dcPhilipsHue()) << "Create flash request" << alert;

    QVariantMap requestMap;
    if (alert == "flash") {
        requestMap.insert("alert", "select");
    } else if (alert == "flash 15 [s]") {
        requestMap.insert("alert", "lselect");
    }
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + hostAddress().toString() + "/api/" + apiKey() +
                                 "/lights/" + QString::number(id()) + "/state"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return QPair<QNetworkRequest, QByteArray>(request, jsonDoc.toJson());
}
