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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "scriptdeviceplugin.h"

#include <QQmlEngine>
#include <QDir>
#include <QJsonDocument>

#include "loggingcategories.h"
#include <plugintimer.h>

ScriptDevicePlugin::ScriptDevicePlugin(QObject *parent) : DevicePlugin(parent)
{

}

bool ScriptDevicePlugin::loadScript(const QString &fileName)
{

    QFileInfo fi(fileName);
    QString metaDataFileName = fi.absoluteDir().path() + '/' + fi.baseName() + ".json";

    QFile metaDataFile(metaDataFileName);
    if (!metaDataFile.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to open plugin metadata at" << metaDataFileName;
        return false;
    }
    QJsonParseError error;
    QByteArray data = metaDataFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    metaDataFile.close();

    if (error.error != QJsonParseError::NoError) {
        int errorOffset = error.offset;
        int newLineIndex = data.indexOf("\n");
        int lineIndex = 1;
        while (newLineIndex > 0 && errorOffset > newLineIndex) {
            data.remove(0, newLineIndex + 2);
            errorOffset -= (newLineIndex + 2);
            newLineIndex = data.indexOf("\n");
            lineIndex++;
        }
        if (newLineIndex >= 0) {
            data = data.left(newLineIndex);
        }
        QString spacer;
        for (int i = 0; i < errorOffset; i++) {
            spacer += ' ';
        }
        QDebug dbg = qWarning(dcDeviceManager()).nospace().noquote();
        dbg << metaDataFileName << ":" << lineIndex << ":" << errorOffset + 2 << ": error: JSON parsing failed: " << error.errorString() << ": " << data.trimmed() << endl;
        dbg << data << endl;
        dbg << spacer << "^";
        return false;
    }
    m_metaData = QJsonObject::fromVariantMap(jsonDoc.toVariant().toMap());

    m_engine = new QQmlEngine(this);
    m_engine->installExtensions(QJSEngine::AllExtensions);

    QJSValue deviceMetaObject = m_engine->newQMetaObject(&Device::staticMetaObject);
    m_engine->globalObject().setProperty("Device", deviceMetaObject);

    m_pluginImport = m_engine->importModule(fileName);
    if (m_pluginImport.isError()) {
        qCWarning(dcDeviceManager()) << "Error loading plugin module" << m_pluginImport.errorType() << m_pluginImport.toString();
        return false;
    }

    return true;
}

QJsonObject ScriptDevicePlugin::metaData() const
{
    return m_metaData;
}

void ScriptDevicePlugin::init()
{
    qmlRegisterType<PluginTimerManager>();
    qmlRegisterType<PluginTimer>();

    QJSValue hardwareManagerObject = m_engine->newQObject(hardwareManager());
    m_engine->globalObject().setProperty("hardwareManager", hardwareManagerObject);

    if (!m_pluginImport.hasOwnProperty("init")) {
        DevicePlugin::init();
        return;
    }
    QJSValue initFunction = m_pluginImport.property("init");
    QJSValue result = initFunction.call();
    if (result.isError()) {
        qCWarning(dcDeviceManager()) << "Error calling init in JS plugin:" << result.toString();
        return;
    }
}

void ScriptDevicePlugin::discoverDevices(DeviceDiscoveryInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("discoverDevices")) {
        DevicePlugin::discoverDevices(info);
        return;
    }

    ScriptDeviceDiscoveryInfo *scriptInfo = new ScriptDeviceDiscoveryInfo(info);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue discoverFunction = m_pluginImport.property("discoverDevices");
    QJSValue ret = discoverFunction.call({jsInfo});
    if (ret.isError()) {
        qCWarning(dcDeviceManager()) << "discoverDevices script failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::startPairing(DevicePairingInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("startPairing")) {
        DevicePlugin::startPairing(info);
        return;
    }

    ScriptDevicePairingInfo *scriptInfo = new ScriptDevicePairingInfo(info);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue startPairingFunction = m_pluginImport.property("startPairing");
    QJSValue ret = startPairingFunction.call({jsInfo});
    if (ret.isError()) {
        qCWarning(dcDeviceManager()) << "startPairing script failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret)
{
    if (!m_pluginImport.hasOwnProperty("confirmPairing")) {
        DevicePlugin::confirmPairing(info, username, secret);
        return;
    }

    ScriptDevicePairingInfo *scriptInfo = new ScriptDevicePairingInfo(info);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue confirmPairingFunction = m_pluginImport.property("confirmPairing");
    QJSValue ret = confirmPairingFunction.call({jsInfo, username, secret});
    if (ret.isError()) {
        qCWarning(dcDeviceManager()) << "confirmPairing script failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::startMonitoringAutoDevices()
{
    if (!m_pluginImport.hasOwnProperty("startMonitoringAutoDevices")) {
        DevicePlugin::startMonitoringAutoDevices();
        return;
    }

    QJSValue monitorFunction = m_pluginImport.property("startMonitoringAutoDevices");
    QJSValue ret = monitorFunction.call();
    if (ret.isError()) {
        qCWarning(dcDeviceManager()) << "startMonitoringAutoDevices failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::setupDevice(DeviceSetupInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("setupDevice")) {
        DevicePlugin::setupDevice(info);
        return;
    }
    QJSValue setupFunction = m_pluginImport.property("setupDevice");

    Device *device = info->device();
    ScriptDevice *scriptDevice = new ScriptDevice(device);
    m_devices.insert(device, scriptDevice);
    connect(device, &Device::destroyed, this, [this, device](){
        m_devices.remove(device);
    });

    ScriptDeviceSetupInfo *scriptInfo = new ScriptDeviceSetupInfo(info, scriptDevice);

    QJSValue jsInfo = m_engine->newQObject(scriptInfo);
    QJSValue ret = setupFunction.call({jsInfo});

    if (ret.errorType() != QJSValue::NoError) {
        qCWarning(dcDeviceManager()) << "setupDevice script failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::postSetupDevice(Device *device)
{
    if (!m_pluginImport.hasOwnProperty("postSetupDevice")) {
        DevicePlugin::postSetupDevice(device);
        return;
    }
    QJSValue postSetupFunction = m_pluginImport.property("postSetupDevice");

    QJSValue jsDevice = m_engine->newQObject(m_devices.value(device));
    QJSValue ret = postSetupFunction.call({jsDevice});
    if (ret.errorType() != QJSValue::NoError) {
        qCWarning(dcDeviceManager()) << "setupDevice script failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::deviceRemoved(Device *device)
{
    if (!m_pluginImport.hasOwnProperty("deviceRemoved")) {
        DevicePlugin::deviceRemoved(device);
        return;
    }

    QJSValue jsDevice = m_engine->newQObject(m_devices.value(device));

    QJSValue deviceRemovedFunction = m_pluginImport.property("deviceRemoved");
    QJSValue ret = deviceRemovedFunction.call({jsDevice});
    if (ret.isError()) {
        qCWarning(dcDeviceManager()) << "deviceRemoved script failed to execute:\n" << ret.toString();
    }
}

void ScriptDevicePlugin::executeAction(DeviceActionInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("executeAction")) {
        DevicePlugin::executeAction(info);
        return;
    }

    ScriptDevice *scriptDevice = m_devices.value(info->device());
    QJSValue jsDevice = m_engine->newQObject(scriptDevice);

    ScriptDeviceActionInfo *scriptInfo = new ScriptDeviceActionInfo(info, scriptDevice);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue executeActionFunction = m_pluginImport.property("executeAction");
    QJSValue ret = executeActionFunction.call({jsInfo});
    if (ret.isError()) {
        qCWarning(dcDeviceManager()) << "executeAction script failed to execute:\n" << ret.toString();
    }
}
