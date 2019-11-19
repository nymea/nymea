#include "scriptengine.h"
#include "devices/devicemanager.h"

#include "scriptaction.h"
#include "scriptevent.h"
#include "scriptstate.h"

#include "nymeasettings.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QJsonParseError>
#include <QJsonDocument>

#include "loggingcategories.h"

namespace nymeaserver {

ScriptEngine::ScriptEngine(DeviceManager *deviceManager, QObject *parent) : QObject(parent),
    m_deviceManager(deviceManager)
{
    qmlRegisterType<ScriptEvent>("nymea", 1, 0, "Event");
    qmlRegisterType<ScriptAction>("nymea", 1, 0, "Action");
    qmlRegisterType<ScriptState>("nymea", 1, 0, "State");

    m_engine = new QQmlApplicationEngine(this);
    m_engine->setProperty("deviceManager", reinterpret_cast<quint64>(m_deviceManager));


    loadScripts();
}

Scripts ScriptEngine::scripts()
{
    Scripts ret;
    foreach (Script *script, m_scripts) {
        ret.append(*script);
    }
    return ret;
}

ScriptEngine::AddScriptReply ScriptEngine::addScript(const QString &name, const QByteArray &content)
{
    QUuid id = QUuid::createUuid();
    QString fileName = baseName(id) + ".qml";
    QString jsonFileName = baseName(id) + ".json";

    AddScriptReply reply;

    QFile jsonFile(jsonFileName);
    if (!jsonFile.open(QFile::ReadWrite)) {
        qCWarning(dcScriptEngine()) << "Error opening script metadata" << jsonFileName;
        reply.scriptError = ScriptErrorHardwareFailure;
        return reply;
    }
    QVariantMap metadata;
    metadata.insert("name", name);
    jsonFile.write(QJsonDocument::fromVariant(metadata).toJson());
    jsonFile.close();

    QFile scriptFile(fileName);
    if (!scriptFile.open(QFile::WriteOnly)) {
        qCWarning(dcScriptEngine()) << "Error opening script file:" << fileName;
        reply.scriptError = ScriptErrorHardwareFailure;
        return reply;
    }

    qint64 len = scriptFile.write(content);
    if (len != content.length()) {
        qCWarning(dcScriptEngine()) << "Error writing script content";
        reply.scriptError = ScriptErrorHardwareFailure;
        return reply;
    }
    scriptFile.close();

    Script *script = new Script();
    script->setId(id);
    script->setName(name);
    bool loaded = loadScript(script);
    if (!loaded) {
        reply.scriptError = ScriptErrorInvalidScript;
        reply.errors = script->errors;
        delete script;
        return reply;
    }

    m_scripts.insert(script->id(), script);

    reply.scriptError = ScriptErrorNoError;
    reply.script = *m_scripts.value(id);
    return reply;
}

ScriptEngine::EditScriptReply ScriptEngine::editScript(const QUuid &id, const QByteArray &content)
{
    QString scriptFileName = baseName(id) + ".qml";
    QFile scriptFile(scriptFileName);
    EditScriptReply reply;

    if (!m_scripts.contains(id)) {
        qCWarning(dcScriptEngine()) << "No script with id" << id;
        reply.scriptError = ScriptErrorScriptNotFound;
        return reply;
    }

    Script *script = m_scripts.value(id);
    unloadScript(script);

    // Deleted compiled qml file to make sure we're reloading the new one
    QString compiledScriptFileName = baseName(id) + ".qmlc";
    QFile::remove(compiledScriptFileName);

    if (!scriptFile.open(QFile::ReadWrite | QFile::Truncate)) {
        qCWarning(dcScriptEngine()) << "Error opening script" << id;
        reply.scriptError = ScriptErrorHardwareFailure;
        return reply;
    }

    QByteArray oldContent = scriptFile.readAll();
    scriptFile.seek(0);

    qint64 bytesWritten = scriptFile.write(content);
    if (bytesWritten != content.length()) {
        qCWarning(dcScriptEngine()) << "Error writing script content";
        reply.scriptError = ScriptErrorHardwareFailure;
        return reply;
    }

    bool loaded = loadScript(script);
    if (!loaded) {
        reply.scriptError = ScriptErrorInvalidScript;
        reply.errors = script->errors;

        // Restore old content
        scriptFile.seek(0);
        scriptFile.write(oldContent);
        loadScript(script);

        return reply;
    }

    reply.scriptError = ScriptErrorNoError;
    return reply;
}

ScriptEngine::ScriptError ScriptEngine::removeScript(const QUuid &id)
{
    Script *script = m_scripts.take(id);
    if (!script) {
        return ScriptErrorScriptNotFound;
    }

    unloadScript(script);

    QString jsonFileName = baseName(id) + ".json";
    QString scriptFileName = baseName(id) + ".qml";
    QString compiledScriptFileName = baseName(id) + ".qmlc";

    QFile::remove(scriptFileName);
    QFile::remove(jsonFileName);
    QFile::remove(compiledScriptFileName);

    delete script;
    return ScriptErrorNoError;
}

void ScriptEngine::loadScripts()
{

//    QString fileName = "/home/micha/Develop/nymea/tests/script.qml";

//    loadScript(fileName);
}

bool ScriptEngine::loadScript(Script *script)
{
    QString fileName = baseName(script->id()) + ".qml";
    QString jsonFileName = baseName(script->id()) + ".json";

    QFile jsonFile(jsonFileName);
    if (!jsonFile.open(QFile::ReadOnly)) {
        qCWarning(dcScriptEngine()) << "Failed to open script metadata";
        return false;
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &error);
    jsonFile.close();

    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcScriptEngine()) << "Failed to parse script metadata";
        return false;
    }

    QString name = jsonDoc.toVariant().toMap().value("name").toString();

    qCWarning(dcScriptEngine()) << "Loading script";

    script->component = new QQmlComponent(m_engine, QUrl::fromLocalFile(fileName), this);
    script->context = new QQmlContext(m_engine, this);
    script->object = script->component->create(script->context);

    if (!script->object) {
        qCWarning(dcScriptEngine()) << "Script failed to load:";
        foreach (const QQmlError &error, script->component->errors()) {
            qCWarning(dcScriptEngine()) << error.toString();
            script->errors.append(QString("%1:%2: %3").arg(error.line()).arg(error.column()).arg(error.description()));
        }
        delete script->context;
        delete script->component;
        return false;
    }
    return true;
}

void ScriptEngine::unloadScript(Script *script)
{
    delete script->object;
    script->object = nullptr;
    delete script->component;
    script->component = nullptr;
    delete script->context;
    script->context = nullptr;
}

QString ScriptEngine::baseName(const QUuid &id)
{
    QString path = NymeaSettings::storagePath() + '/';
    QString basename = id.toString().remove(QRegExp("[{}]"));
    return path + basename;
}

}
