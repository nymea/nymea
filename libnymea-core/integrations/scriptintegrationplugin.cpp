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

#include "scriptintegrationplugin.h"

#include <QQmlEngine>
#include <QDir>
#include <QJsonDocument>

#include "loggingcategories.h"
#include <plugintimer.h>

ScriptIntegrationPlugin::ScriptIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

bool ScriptIntegrationPlugin::loadScript(const QString &fileName)
{

    QFileInfo fi(fileName);
    QString metaDataFileName = fi.absoluteDir().path() + '/' + fi.baseName() + ".json";

    QFile metaDataFile(metaDataFileName);
    if (!metaDataFile.open(QFile::ReadOnly)) {
        qCWarning(dcThingManager()) << "Failed to open plugin metadata at" << metaDataFileName;
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
        QDebug dbg = qWarning(dcThingManager()).nospace().noquote();
        dbg << metaDataFileName << ":" << lineIndex << ":" << errorOffset + 2 << ": error: JSON parsing failed: " << error.errorString() << ": " << data.trimmed() << endl;
        dbg << data << endl;
        dbg << spacer << "^";
        return false;
    }
    m_metaData = QJsonObject::fromVariantMap(jsonDoc.toVariant().toMap());

    m_engine = new QQmlEngine(this);
    m_engine->installExtensions(QJSEngine::AllExtensions);

    QJSValue thingMetaObject = m_engine->newQMetaObject(&Thing::staticMetaObject);
    m_engine->globalObject().setProperty("Thing", thingMetaObject);

    m_pluginImport = m_engine->importModule(fileName);
    if (m_pluginImport.isError()) {
        qCWarning(dcThingManager()) << "Error loading plugin module" << m_pluginImport.errorType() << m_pluginImport.toString();
        return false;
    }

    return true;
}

QJsonObject ScriptIntegrationPlugin::metaData() const
{
    return m_metaData;
}

void ScriptIntegrationPlugin::init()
{
    //Couldn't find an non-qml way to register abstract classes in the JS engine as qRegisterMetatype doesn't deal so well with abstract classes
    qmlRegisterUncreatableType<PluginTimerManager>("nymea", 1, 0, "PluginTimerManager", "Get it from hardwareManager");
    qmlRegisterUncreatableType<PluginTimer>("nymea", 1, 0, "PluginTimer", "Get it from PluginTimerManager");

    QJSValue hardwareManagerObject = m_engine->newQObject(hardwareManager());
    m_engine->globalObject().setProperty("hardwareManager", hardwareManagerObject);

    if (!m_pluginImport.hasOwnProperty("init")) {
        IntegrationPlugin::init();
        return;
    }
    QJSValue initFunction = m_pluginImport.property("init");
    QJSValue result = initFunction.call();
    if (result.isError()) {
        qCWarning(dcThingManager()) << "Error calling init in JS plugin:" << result.toString();
        return;
    }
}

void ScriptIntegrationPlugin::discoverThings(ThingDiscoveryInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("discoverThings")) {
        IntegrationPlugin::discoverThings(info);
        return;
    }

    ScriptThingDiscoveryInfo *scriptInfo = new ScriptThingDiscoveryInfo(info);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue discoverFunction = m_pluginImport.property("discoverThings");
    QJSValue ret = discoverFunction.call({jsInfo});
    if (ret.isError()) {
        qCWarning(dcThingManager()) << "discoverThings script failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::startPairing(ThingPairingInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("startPairing")) {
        IntegrationPlugin::startPairing(info);
        return;
    }

    ScriptThingPairingInfo *scriptInfo = new ScriptThingPairingInfo(info);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue startPairingFunction = m_pluginImport.property("startPairing");
    QJSValue ret = startPairingFunction.call({jsInfo});
    if (ret.isError()) {
        qCWarning(dcThingManager()) << "startPairing script failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret)
{
    if (!m_pluginImport.hasOwnProperty("confirmPairing")) {
        IntegrationPlugin::confirmPairing(info, username, secret);
        return;
    }

    ScriptThingPairingInfo *scriptInfo = new ScriptThingPairingInfo(info);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue confirmPairingFunction = m_pluginImport.property("confirmPairing");
    QJSValue ret = confirmPairingFunction.call({jsInfo, username, secret});
    if (ret.isError()) {
        qCWarning(dcThingManager()) << "confirmPairing script failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::startMonitoringAutoThings()
{
    if (!m_pluginImport.hasOwnProperty("startMonitoringAutoThings")) {
        IntegrationPlugin::startMonitoringAutoThings();
        return;
    }

    QJSValue monitorFunction = m_pluginImport.property("startMonitoringAutoThings");
    QJSValue ret = monitorFunction.call();
    if (ret.isError()) {
        qCWarning(dcThingManager()) << "startMonitoringAutoThings failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::setupThing(ThingSetupInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("setupThing")) {
        IntegrationPlugin::setupThing(info);
        return;
    }
    QJSValue setupFunction = m_pluginImport.property("setupThing");

    Thing *thing = info->thing();
    ScriptThing *scriptThing = new ScriptThing(thing);
    m_things.insert(thing, scriptThing);
    connect(thing, &Thing::destroyed, this, [this, thing](){
        m_things.remove(thing);
    });

    ScriptThingSetupInfo *scriptInfo = new ScriptThingSetupInfo(info, scriptThing);

    QJSValue jsInfo = m_engine->newQObject(scriptInfo);
    QJSValue ret = setupFunction.call({jsInfo});

    if (ret.errorType() != QJSValue::NoError) {
        qCWarning(dcThingManager()) << "setupThing script failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::postSetupThing(Thing *thing)
{
    if (!m_pluginImport.hasOwnProperty("postSetupThing")) {
        IntegrationPlugin::postSetupThing(thing);
        return;
    }
    QJSValue postSetupFunction = m_pluginImport.property("postSetupThing");

    QJSValue jsThing = m_engine->newQObject(m_things.value(thing));
    QJSValue ret = postSetupFunction.call({jsThing});
    if (ret.errorType() != QJSValue::NoError) {
        qCWarning(dcThingManager()) << "setupThing script failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::thingRemoved(Thing *thing)
{
    if (!m_pluginImport.hasOwnProperty("thingRemoved")) {
        IntegrationPlugin::thingRemoved(thing);
        return;
    }

    QJSValue jsThing = m_engine->newQObject(m_things.value(thing));

    QJSValue thingRemovedFunction = m_pluginImport.property("thingRemoved");
    QJSValue ret = thingRemovedFunction.call({jsThing});
    if (ret.isError()) {
        qCWarning(dcThingManager()) << "thingRemoved script failed to execute:\n" << ret.toString();
    }
}

void ScriptIntegrationPlugin::executeAction(ThingActionInfo *info)
{
    if (!m_pluginImport.hasOwnProperty("executeAction")) {
        IntegrationPlugin::executeAction(info);
        return;
    }

    ScriptThing *scriptThing = m_things.value(info->thing());
    QJSValue jsThing = m_engine->newQObject(scriptThing);

    ScriptThingActionInfo *scriptInfo = new ScriptThingActionInfo(info, scriptThing);
    QJSValue jsInfo = m_engine->newQObject(scriptInfo);

    QJSValue executeActionFunction = m_pluginImport.property("executeAction");
    QJSValue ret = executeActionFunction.call({jsInfo});
    if (ret.isError()) {
        qCWarning(dcThingManager()) << "executeAction script failed to execute:\n" << ret.toString();
    }
}
