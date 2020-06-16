/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef LOGGINGCATEGORYS_H
#define LOGGINGCATEGORYS_H

#include <QLoggingCategory>
#include <QDebug>

extern QStringList s_nymeaLoggingCategories;


#define NYMEA_LOGGING_CATEGORY(name, string) \
    class NymeaLoggingCategory##name: public QLoggingCategory { \
    public: \
    NymeaLoggingCategory##name(): QLoggingCategory(string) { s_nymeaLoggingCategories.append(string); } \
    }; \
    static NymeaLoggingCategory##name s_##name; \
    const QLoggingCategory &name() \
    { \
        return s_##name; \
    } \

// FIXME: Those should eventually disappear from here
Q_DECLARE_LOGGING_CATEGORY(dcPluginMetadata)
Q_DECLARE_LOGGING_CATEGORY(dcThing)
Q_DECLARE_LOGGING_CATEGORY(dcThingManager)
Q_DECLARE_LOGGING_CATEGORY(dcSystem)
Q_DECLARE_LOGGING_CATEGORY(dcPlatform)
Q_DECLARE_LOGGING_CATEGORY(dcPlatformUpdate)
Q_DECLARE_LOGGING_CATEGORY(dcPlatformZeroConf)
Q_DECLARE_LOGGING_CATEGORY(dcExperiences)
Q_DECLARE_LOGGING_CATEGORY(dcTimeManager)
Q_DECLARE_LOGGING_CATEGORY(dcRuleEngine)
Q_DECLARE_LOGGING_CATEGORY(dcRuleEngineDebug)
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)
Q_DECLARE_LOGGING_CATEGORY(dcHardware)
Q_DECLARE_LOGGING_CATEGORY(dcLogEngine)
Q_DECLARE_LOGGING_CATEGORY(dcServerManager)
Q_DECLARE_LOGGING_CATEGORY(dcTcpServer)
Q_DECLARE_LOGGING_CATEGORY(dcTcpServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcWebServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcDebugServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpcTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcRest)
Q_DECLARE_LOGGING_CATEGORY(dcOAuth2)
Q_DECLARE_LOGGING_CATEGORY(dcUpnp)
Q_DECLARE_LOGGING_CATEGORY(dcBluetooth)
Q_DECLARE_LOGGING_CATEGORY(dcCloud)
Q_DECLARE_LOGGING_CATEGORY(dcCloudTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcNetworkManager)
Q_DECLARE_LOGGING_CATEGORY(dcUserManager)
Q_DECLARE_LOGGING_CATEGORY(dcAWS)
Q_DECLARE_LOGGING_CATEGORY(dcAWSTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcBluetoothServer)
Q_DECLARE_LOGGING_CATEGORY(dcBluetoothServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcMqtt)
Q_DECLARE_LOGGING_CATEGORY(dcTranslations)
Q_DECLARE_LOGGING_CATEGORY(dcCoap)
Q_DECLARE_LOGGING_CATEGORY(dcI2C)


/*
  Installs a nymea log message handler in the system.
  This is different to the qLogMessageHandler which works like a chain.

  In nymea we have the use case that we need to copy log messages
  to different places and also dynamically install/uninstall such
  handlers.

  If you need to copy log messages, use this, if you need to modify log messages
  for the entire system (e.g. redirect to a different logging category, the Qt's
  mechanism of qInstallMessageHandler() is still available and will always be called
  *before* distributing the message to every nymea message handler.
*/

void nymeaInstallMessageHandler(QtMessageHandler handler);
void nymeaUninstallMessageHandler(QtMessageHandler handler);
bool initLogging(const QString &fileName, bool useColors);
void closeLogFile();

#endif // LOGGINGCATEGORYS_H
