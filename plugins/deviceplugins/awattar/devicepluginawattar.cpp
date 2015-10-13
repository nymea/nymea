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

/*!
    \page awattar.html
    \title aWATTar

    \ingroup plugins
    \ingroup network

    This plugin allows to receive the current energy market price from the \l{https://www.awattar.com/}{aWATTar GmbH}.
    In order to use this plugin you need to enter the access token from your energy provider. You can find more
    information about you accesstoken \l{https://www.awattar.com/api-unser-datenfeed}{here}.

    The data will be updated every hour. The API allows a maximum of 100 calls per day.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/awattar/devicepluginawattar.json
*/

#include "devicepluginawattar.h"
#include "plugin/device.h"
#include "plugininfo.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QSslConfiguration>

DevicePluginAwattar::DevicePluginAwattar()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(60000);

    connect(m_timer, &QTimer::timeout, this, &DevicePluginAwattar::onTimeout);
}

DeviceManager::HardwareResources DevicePluginAwattar::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceSetupStatus DevicePluginAwattar::setupDevice(Device *device)
{
    QString token = device->paramValue("token").toString();
    qCDebug(dcAwattar) << "Setup device with token" << token;

    QNetworkReply *reply = requestData(token);
    m_asyncSetup.insert(reply, device);

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginAwattar::deviceRemoved(Device *device)
{
    Q_UNUSED(device)

    if (myDevices().isEmpty()) {
        m_timer->stop();
    }
}

void DevicePluginAwattar::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // create user finished
    if (m_asyncSetup.keys().contains(reply)) {
        Device *device = m_asyncSetup.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcAwattar) << "Setup reply HTTP error:" << status << reply->errorString();
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            reply->deleteLater();
            return;
        }

        // check JSON file
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcAwattar) << "Setup reply JSON error:" << error.errorString();
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            reply->deleteLater();
            return;
        }

        processData(device, jsonDoc.toVariant().toMap(), true);
    } else if (m_update.keys().contains(reply)) {
        Device *device = m_update.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcAwattar) << "Update reply HTTP error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }

        // check JSON file
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcAwattar) << "Update reply JSON error:" << error.errorString();
            reply->deleteLater();
            return;
        }

        processData(device, jsonDoc.toVariant().toMap());
    }
    reply->deleteLater();
}

void DevicePluginAwattar::processData(Device *device, const QVariantMap &data, const bool &fromSetup)
{
    if (!data.contains("data")) {
        if (fromSetup) {
            qCWarning(dcAwattar) << "Device setup failed." << device->id().toString() << "No data element received";
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            return;
        }
        qCWarning(dcAwattar) << "Update failed for device" << device->id().toString() << "No data element received";
        return;
    }

    QVariantList dataElements = data.value("data").toList();

    QDateTime currentTime = QDateTime::currentDateTime();
    foreach (QVariant element, dataElements) {
        QVariantMap elementMap = element.toMap();
        QDateTime startTime = QDateTime::fromMSecsSinceEpoch((qint64)elementMap.value("start_timestamp").toLongLong());
        QDateTime endTime = QDateTime::fromMSecsSinceEpoch((qint64)elementMap.value("end_timestamp").toLongLong());
        double marketPrice = elementMap.value("marketprice").toDouble();
        if (currentTime  >= startTime && currentTime <= endTime) {
            qCDebug(dcAwattar) << "---------------------------------------";
            qCDebug(dcAwattar) << "start  :" << startTime.toString();
            qCDebug(dcAwattar) << "end    :" << endTime.toString();
            qCDebug(dcAwattar) << "price  :" << marketPrice << elementMap.value("unit").toString();
            device->setStateValue(currentMarketPriceStateTypeId, marketPrice);
            device->setStateValue(validUntilStateTypeId, endTime.toLocalTime().toTime_t());
        }
    }

    if (fromSetup) {
        m_timer->start();
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
    }
}

QNetworkReply *DevicePluginAwattar::requestData(const QString &token)
{
    QByteArray data = QString(token + ":").toUtf8().toBase64();
    QString header = "Basic " + data;
    QNetworkRequest request(QUrl("https://api.awattar.com/v1/marketdata"));
    request.setRawHeader("Authorization", header.toLocal8Bit());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    return networkManagerGet(request);
}

void DevicePluginAwattar::updateDevice(Device *device)
{
    QNetworkReply *reply = requestData(device->paramValue("token").toString());
    m_update.insert(reply, device);
}

void DevicePluginAwattar::onTimeout()
{
    // check every hour
    if(QDateTime::currentDateTime().time().minute() == 0) {
        foreach (Device *device, myDevices()) {
            qCDebug(dcAwattar) << "Update device" << device->id().toString();
            updateDevice(device);
        }
    }
}

