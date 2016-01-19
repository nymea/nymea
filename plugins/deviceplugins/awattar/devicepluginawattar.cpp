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

    The pricing data will be updated every hour.

    \chapter Available data

    In following chart you can see an example of the market prices from -12 hours to + 12 hours from the current
    time (0).The green line describes the current market price, the red point line describes the average
    price of this interval and the red line describes the deviation. If the deviation is positiv, the current
    price is above the average, if the deviation is negative, the current price is below the average.

    \list
        \li -100 % \unicode{0x2192} current price equals lowest price in the interval [-12h < now < + 12h]
        \li 0 %    \unicode{0x2192} current price equals average price in the interval  [-12h < now < + 12h]
        \li +100 % \unicode{0x2192} current price equals highest price in the interval [-12h < now < + 12h]
    \endlist

    \image awattar-graph.png

    Information about the smart grid modes can be found \l{https://www.waermepumpe.de/sg-ready/}{here}.

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

}

DeviceManager::HardwareResources DevicePluginAwattar::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginAwattar::setupDevice(Device *device)
{
    if (!myDevices().isEmpty()) {
        qCWarning(dcAwattar) << "Only one aWATTar device can be configured.";
    }

    QString token = device->paramValue("token").toString();
    qCDebug(dcAwattar) << "Setup device" << device->params();

    QNetworkReply *reply = requestPriceData(token);
    m_asyncSetup.insert(reply, device);

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginAwattar::startMonitoringAutoDevices()
{
    searchHeatPumps();
}

void DevicePluginAwattar::deviceRemoved(Device *device)
{
    Q_UNUSED(device)

    foreach (HeatPump *pump, m_heatPumps) {
        qCDebug(dcAwattar) << "Delete pump" << pump->address().toString();
        pump->deleteLater();
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

        processPriceData(device, jsonDoc.toVariant().toMap(), true);

        QNetworkReply *userReply = requestUserData(device->paramValue("token").toString(), device->paramValue("user uuid").toString());
        m_updateUserData.insert(userReply, device);

    } else if (m_updatePrice.keys().contains(reply)) {
        Device *device = m_updatePrice.take(reply);

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

        processPriceData(device, jsonDoc.toVariant().toMap());

        QNetworkReply *userReply = requestUserData(device->paramValue("token").toString(), device->paramValue("user uuid").toString());
        m_updateUserData.insert(userReply, device);

    } else if (m_updateUserData.keys().contains(reply)) {
        Device *device = m_updateUserData.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcAwattar) << "Update user data reply HTTP error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }

        // check JSON file
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcAwattar) << "Update user data reply JSON error:" << error.errorString();
            reply->deleteLater();
            return;
        }

        processUserData(device, jsonDoc.toVariant().toMap());

    } else if (m_searchPumpReplies.contains(reply)) {

        m_searchPumpReplies.removeAll(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcAwattar) << "Search pump reply HTTP error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }

        processPumpSearchData(reply->readAll());
    }

    reply->deleteLater();
}

void DevicePluginAwattar::guhTimer()
{
    foreach (Device *device, myDevices()) {
        //qCDebug(dcAwattar) << "Update device" << device->id().toString();
        searchHeatPumps();
        updateDevice(device);
    }
}

DeviceManager::DeviceError DevicePluginAwattar::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device)
    Q_UNUSED(action)

//    if (action.actionTypeId() == ledPowerActionTypeId) {
//        foreach (HeatPump *pump, m_heatPumps) {
//            if (!pump->reachable())
//                return DeviceManager::DeviceErrorHardwareNotAvailable;

//            pump->setLed(action.param("led power").value().toBool());
//        }
//    } else if (action.actionTypeId() == sgModeActionTypeId) {
//        foreach (HeatPump *pump, m_heatPumps) {
//            if (!pump->reachable())
//                return DeviceManager::DeviceErrorHardwareNotAvailable;

//            pump->setSgMode(action.param("sg-mode").value().toInt());
//        }
//    }

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginAwattar::processPriceData(Device *device, const QVariantMap &data, const bool &fromSetup)
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
    double sum = 0;
    double count = 0;
    double averagePrice = 0;
    double currentPrice = 0;
    int deviation = 0;
    double maxPrice = -1000;
    double minPrice = 1000;
    foreach (QVariant element, dataElements) {
        QVariantMap elementMap = element.toMap();
        QDateTime startTime = QDateTime::fromMSecsSinceEpoch((qint64)elementMap.value("start_timestamp").toLongLong());
        QDateTime endTime = QDateTime::fromMSecsSinceEpoch((qint64)elementMap.value("end_timestamp").toLongLong());
        double price = elementMap.value("marketprice").toDouble();

        // check interval [-12h < x < + 12h]
        if ((startTime >= currentTime.addSecs(-3600 * 12) && endTime <= currentTime ) ||
                (endTime <= currentTime.addSecs(3600 * 12) && startTime >= currentTime )) {
            sum += price;
            count++;

            if (price > maxPrice)
                maxPrice = price;

            if (price < minPrice)
                minPrice = price;
        }

        if (currentTime  >= startTime && currentTime <= endTime) {
            currentPrice = price;
            sum += price;
            count++;

            if (price > maxPrice)
                maxPrice = price;

            if (price < minPrice)
                minPrice = price;

            //qCDebug(dcAwattar) << startTime.toString() << " -> " << endTime.toString();
            device->setStateValue(currentMarketPriceStateTypeId, currentPrice / 10.0);
            device->setStateValue(validUntilStateTypeId, endTime.toLocalTime().toTime_t());
        }
    }

    // calculate averagePrice and mean deviation
    averagePrice = sum / count;

    if (currentPrice <= averagePrice) {
        deviation = -1 * qRound(100 + (-100 * (currentPrice - minPrice) / (averagePrice - minPrice)));
    } else {
        deviation = qRound(-100 * (averagePrice - currentPrice) / (maxPrice - averagePrice));
    }

//    qCDebug(dcAwattar) << "    price    :" << currentPrice << "Eur/MWh";
//    qCDebug(dcAwattar) << "    average  :" << averagePrice << "Eur/MWh";
//    qCDebug(dcAwattar) << "    deviation:" << deviation << "%";
//    qCDebug(dcAwattar) << "    min      :" << minPrice << "Eur/MWh";
//    qCDebug(dcAwattar) << "    max      :" << maxPrice << "Eur/MWh";

    device->setStateValue(averagePriceStateTypeId, averagePrice / 10.0);
    device->setStateValue(lowestPriceStateTypeId, minPrice / 10.0);
    device->setStateValue(highestPriceStateTypeId, maxPrice / 10.0);
    device->setStateValue(averageDeviationStateTypeId, deviation);

    if (fromSetup)
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);

}

void DevicePluginAwattar::processUserData(Device *device, const QVariantMap &data)
{
    QVariantList dataElements = data.value("data").toList();

    QDateTime currentTime = QDateTime::currentDateTime();
    foreach (QVariant element, dataElements) {
        QVariantMap elementMap = element.toMap();
        QDateTime startTime = QDateTime::fromMSecsSinceEpoch((qint64)elementMap.value("start_timestamp").toLongLong());
        QDateTime endTime = QDateTime::fromMSecsSinceEpoch((qint64)elementMap.value("end_timestamp").toLongLong());

        // check if we are in the current interval
        if (currentTime >= startTime && currentTime <= endTime) {
            int sgMode = 0;
            if (elementMap.contains("data")) {
                if (elementMap.value("data").toMap().contains("sg-mode")) {
                    sgMode = elementMap.value("data").toMap().value("sg-mode").toInt();
                }
            }

            switch (sgMode) {
            case 1:
                device->setStateValue(sgModeStateTypeId, "1 - Off");
                break;
            case 2:
                device->setStateValue(sgModeStateTypeId, "2 - Normal");
                break;
            case 3:
                device->setStateValue(sgModeStateTypeId, "3 - High Temperature");
                break;
            case 4:
                device->setStateValue(sgModeStateTypeId, "4 - On");
                break;
            default:
                device->setStateValue(sgModeStateTypeId, "0 - Invalid");
                continue;
            }

            foreach (HeatPump *pump, m_heatPumps) {
                pump->setSgMode(sgMode);
            }
        }
    }
}

void DevicePluginAwattar::processPumpSearchData(const QByteArray &data)
{
    //qCDebug(dcAwattar) << "Search result:" << endl << data;

    QList<QByteArray> lines = data.split('\n');
    foreach (const QByteArray &line, lines) {
        if (line.isEmpty())
            continue;

        // remove the '/128' from the address
        QHostAddress pumpAddress(QString(data.left(line.length() - 4)));
        if (!pumpAddress.isNull()) {

            // check if we already created this heat pump
            if (heatPumpExists(pumpAddress))
                continue;

            qCDebug(dcAwattar) << "Found heat pump at" << pumpAddress.toString();

            HeatPump *pump = new HeatPump(pumpAddress, this);
            connect(pump, SIGNAL(reachableChanged()), this, SLOT(onHeatPumpReachableChanged()));

            m_heatPumps.append(pump);

        } else {
            qCWarning(dcAwattar) << "Could not read pump address" << line;
        }
    }
}

QNetworkReply *DevicePluginAwattar::requestPriceData(const QString &token)
{
    QByteArray data = QString(token + ":").toUtf8().toBase64();
    QString header = "Basic " + data;
    QNetworkRequest request(QUrl("https://api.awattar.com/v1/marketdata"));
    request.setRawHeader("Authorization", header.toLocal8Bit());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    return networkManagerGet(request);
}

QNetworkReply *DevicePluginAwattar::requestUserData(const QString &token, const QString &userId)
{
    QByteArray data = QString(token + ":").toUtf8().toBase64();
    QString header = "Basic " + data;

    QNetworkRequest request(QUrl(QString("https://api.awattar.com/v1/devices/%1/actuators").arg(userId)));
    request.setRawHeader("Authorization", header.toLocal8Bit());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    return networkManagerGet(request);
}

void DevicePluginAwattar::updateDevice(Device *device)
{
    QNetworkReply *priceReply = requestPriceData(device->paramValue("token").toString());
    m_updatePrice.insert(priceReply, device);
}

void DevicePluginAwattar::searchHeatPumps()
{
    QHostAddress rplAddress = QHostAddress(configuration().paramValue("RPL address").toString());

    if (rplAddress.isNull()) {
        qCWarning(dcAwattar) << "Invalid RPL address" << configuration().paramValue("RPL address").toString();
        return;
    }

    //qCDebug(dcAwattar) << "Search heat pump" << rplAddress.toString();

    QNetworkRequest request(QUrl(QString("http://[%1]").arg(rplAddress.toString())));
    QNetworkReply *reply = networkManagerGet(request);

    m_searchPumpReplies.append(reply);
}

bool DevicePluginAwattar::heatPumpExists(const QHostAddress &pumpAddress)
{
    foreach (HeatPump *pump, m_heatPumps) {
        if (pump->address() == pumpAddress) {
            return true;
        }
    }
    return false;
}

void DevicePluginAwattar::onHeatPumpReachableChanged()
{
    HeatPump *pump = static_cast<HeatPump *>(sender());

    foreach (Device *device, myDevices()) {
        device->setStateValue(reachableStateTypeId, pump->reachable());
    }
}
