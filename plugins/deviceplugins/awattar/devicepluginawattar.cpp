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

    \chapter Heat pump

    Information about the smart grid modes can be found \l{https://www.waermepumpe.de/sg-ready/}{here}.

    In order to interact with the heat pump (SG-ready), this plugin creates a CoAP connection to the server running on the
    6LoWPAN bridge. The server IPv6 can be configured in the plugin configuration. Once the connection is established, the
    plugin searches for 6LoWPAN neighbors in the network.

    \note Currently there should be only one heat pump in the 6LoWPAN network!

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

DevicePluginAwattar::DevicePluginAwattar() :
    m_device(0),
    m_setupRetry(0)
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
        return DeviceManager::DeviceSetupStatusFailure;
    }

    qCDebug(dcAwattar) << "Setup device" << device->name() << device->params();

    m_device = device;
    m_token = device->paramValue("token").toString();
    m_userUuid = device->paramValue("user uuid").toString();

    connectionTest();

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginAwattar::deviceRemoved(Device *device)
{
    Q_UNUSED(device)

    foreach (HeatPump *pump, m_heatPumps) {
        qCDebug(dcAwattar) << "Delete pump" << pump->address().toString();
        pump->deleteLater();
    }

    m_device = 0;
}

void DevicePluginAwattar::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_asyncSetup.contains(reply)) {

        m_asyncSetup.removeAll(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcAwattar) << "Setup reply HTTP error:" << status << reply->errorString();
            if (m_setupRetry == 3) {
                emit deviceSetupFinished(m_device, DeviceManager::DeviceSetupStatusFailure);
                m_setupRetry = 0;
                reply->deleteLater();
                return;
            } else {
                m_setupRetry++;
                reply->deleteLater();

                // retry in 1 sec
                qCWarning(dcAwattar) << "Retry to connect" << m_setupRetry << "/ 3";
                QTimer::singleShot(2000, this, SLOT(connectionTest()));
                return;
            }
        }

        // check JSON file
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcAwattar) << "Setup reply JSON error:" << error.errorString();
            emit deviceSetupFinished(m_device, DeviceManager::DeviceSetupStatusFailure);
            reply->deleteLater();
            return;
        }

        Q_UNUSED(jsonDoc)

        emit deviceSetupFinished(m_device, DeviceManager::DeviceSetupStatusSuccess);

        // get data
        searchHeatPumps();
        updateData();

    } else if (m_updatePrice.contains(reply)) {

        m_updatePrice.removeAll(reply);

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

        processPriceData(jsonDoc.toVariant().toMap());

        // start user data update
        m_updateUserData.append(requestUserData(m_token, m_userUuid));

    } else if (m_updateUserData.contains(reply)) {

        m_updateUserData.removeAll(reply);

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

        processUserData(jsonDoc.toVariant().toMap());

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
    if (!m_device)
        return;

    updateData();
    searchHeatPumps();
}

DeviceManager::DeviceError DevicePluginAwattar::executeAction(Device *device, const Action &action)
{
    if (!m_device || m_device != device)
        return DeviceManager::DeviceErrorHardwareNotAvailable;

    if (action.actionTypeId() == sgSyncModeActionTypeId) {
        qCDebug(dcAwattar) << "Set sg sync mode to" << action.param("sync mode").value();
        device->setStateValue(sgSyncModeStateTypeId, action.param("sync mode").value());
        if (action.param("sync mode").value() == "auto")
            setSgMode(m_autoSgMode);

        return DeviceManager::DeviceErrorNoError;
    } else if (action.actionTypeId() == setSgModeActionTypeId) {
        if (!device->stateValue(reachableStateTypeId).toBool()) {
            qCWarning(dcAwattar) << "Could not set SG mode. The pump is not reachable";
            return DeviceManager::DeviceErrorHardwareNotAvailable;
        }

        device->setStateValue(sgSyncModeStateTypeId, "manual");
        QString sgModeString = action.param("sg-mode").value().toString();
        qCDebug(dcAwattar) << "Set manual SG mode to:" << sgModeString;

        if(sgModeString == "1 - Off") {
            m_manualSgMode = 1;
        } else if (sgModeString == "2 - Normal") {
            m_manualSgMode = 2;
        } else if (sgModeString == "3 - High Temperature") {
            m_manualSgMode = 3;
        } else if (sgModeString == "4 - On") {
            m_manualSgMode = 4;
        }

        setSgMode(m_manualSgMode);
        return DeviceManager::DeviceErrorNoError;
    }

    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginAwattar::processPriceData(const QVariantMap &data)
{
    if (!m_device)
        return;

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

            m_device->setStateValue(currentMarketPriceStateTypeId, currentPrice / 10.0);
            m_device->setStateValue(validUntilStateTypeId, endTime.toLocalTime().toTime_t());
        }
    }

    // calculate averagePrice and mean deviation
    averagePrice = sum / count;

    if (currentPrice <= averagePrice) {
        deviation = -1 * qRound(100 + (-100 * (currentPrice - minPrice) / (averagePrice - minPrice)));
    } else {
        deviation = qRound(-100 * (averagePrice - currentPrice) / (maxPrice - averagePrice));
    }

    m_device->setStateValue(averagePriceStateTypeId, averagePrice / 10.0);
    m_device->setStateValue(lowestPriceStateTypeId, minPrice / 10.0);
    m_device->setStateValue(highestPriceStateTypeId, maxPrice / 10.0);
    m_device->setStateValue(averageDeviationStateTypeId, deviation);
}

void DevicePluginAwattar::processUserData(const QVariantMap &data)
{
    if (!m_device)
        return;

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

            m_autoSgMode = sgMode;

            // sync the sg mode to each pump available
            if (m_device->stateValue(sgSyncModeStateTypeId).toString() == "auto") {
                setSgMode(m_autoSgMode);
            } else {
                setSgMode(m_manualSgMode);
            }
        }
    }
}

void DevicePluginAwattar::processPumpSearchData(const QByteArray &data)
{
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

void DevicePluginAwattar::updateData()
{
    m_updatePrice.append(requestPriceData(m_token));
}

void DevicePluginAwattar::searchHeatPumps()
{
    QHostAddress rplAddress = QHostAddress(configuration().paramValue("RPL address").toString());

    if (rplAddress.isNull()) {
        qCWarning(dcAwattar) << "Invalid RPL address" << configuration().paramValue("RPL address").toString();
        return;
    }

    QNetworkRequest request(QUrl(QString("http://[%1]").arg(rplAddress.toString())));
    QNetworkReply *reply = networkManagerGet(request);

    m_searchPumpReplies.append(reply);
}

void DevicePluginAwattar::setSgMode(const int &sgMode)
{
    switch (sgMode) {
    case 1:
        m_device->setStateValue(sgModeStateTypeId, "1 - Off");
        break;
    case 2:
        m_device->setStateValue(sgModeStateTypeId, "2 - Normal");
        break;
    case 3:
        m_device->setStateValue(sgModeStateTypeId, "3 - High Temperature");
        break;
    case 4:
        m_device->setStateValue(sgModeStateTypeId, "4 - On");
        break;
    default:
        m_device->setStateValue(sgModeStateTypeId, "0 - Invalid");
        return;
    }

    foreach (HeatPump *pump, m_heatPumps) {
        if (pump->reachable()) {
            pump->setSgMode(sgMode);
        }
    }
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

void DevicePluginAwattar::connectionTest()
{
    m_asyncSetup.append(requestUserData(m_token, m_userUuid));
}

void DevicePluginAwattar::onHeatPumpReachableChanged()
{
    HeatPump *pump = static_cast<HeatPump *>(sender());
    qCDebug(dcAwattar) << "Heatpump" << pump->address().toString() << " -> reachable" << pump->reachable();

    // if there is still one pump reachable, lets keep the state true
    // if no pump can be reached, the state will be set to false
    bool reachable = false;
    foreach (HeatPump *heatPump, m_heatPumps) {
        if (heatPump->reachable()){
            reachable = true;
            break;
        }
    }

    foreach (Device *device, myDevices()) {
        device->setStateValue(reachableStateTypeId, reachable);
    }
}
