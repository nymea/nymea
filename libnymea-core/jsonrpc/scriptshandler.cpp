#include "scriptshandler.h"

#include "loggingcategories.h"

#include "scriptengine/scriptengine.h"

namespace nymeaserver {

ScriptsHandler::ScriptsHandler(ScriptEngine *scriptEngine, QObject *parent):
    JsonHandler(parent),
    m_engine(scriptEngine)
{
    registerEnum<ScriptEngine::ScriptError>();
    registerEnum<ScriptEngine::ScriptMessageType>();

    registerObject<Script, Scripts>();

    QVariantMap params, returns;
    QString description;

    params.clear(); returns.clear();
    description = "Get all script, that is, their names and properties, but no content.";
    returns.insert("scripts", objectRef<Scripts>());
    registerMethod("GetScripts", description, params, returns);

    params.clear(); returns.clear();
    description = "Get a scripts content.";
    params.insert("id", enumValueName(Uuid));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    returns.insert("o:content", enumValueName(String));
    registerMethod("GetScriptContent", description, params, returns);

    params.clear(); returns.clear();
    description = "Add a script";
    params.insert("name", enumValueName(String));
    params.insert("content", enumValueName(String));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    returns.insert("o:script", objectRef<Script>());
    returns.insert("o:errors", enumValueName(StringList));
    registerMethod("AddScript", description, params, returns);

    params.clear(); returns.clear();
    description = "Edit a script";
    params.insert("id", enumValueName(Uuid));
    params.insert("o:name", enumValueName(String));
    params.insert("content", enumValueName(String));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    returns.insert("o:errors", enumValueName(StringList));
    registerMethod("EditScript", description, params, returns);

    params.clear(); returns.clear();
    description = "remove a script";
    params.insert("id", enumValueName(Uuid));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    registerMethod("RemoveScript", description, params, returns);

    params.clear();
    description = "Emitted when a script has been added to the system.";
    params.insert("script", objectRef<Script>());
    registerNotification("ScriptAdded", description, params);

    params.clear();
    description = "Emitted when a script has been removed from the system.";
    params.insert("id", enumValueName(Uuid));
    registerNotification("ScriptRemoved", description, params);

    params.clear();
    description = "Emitted when a script has been changed in the system.";
    params.insert("script", objectRef<Script>());
    registerNotification("ScriptChanged", description, params);

    params.clear();
    description = "Emitted when a script produces a console message.";
    params.insert("scriptId", enumValueName(Uuid));
    params.insert("type", enumRef<ScriptEngine::ScriptMessageType>());
    params.insert("message", enumValueName(String));
    registerNotification("ScriptLogMessage", description, params);

    connect(m_engine, &ScriptEngine::scriptAdded, this, [this](const Script &script){
        QVariantMap params;
        params.insert("script", pack(script));
        emit ScriptAdded(params);
    });
    connect(m_engine, &ScriptEngine::scriptRemoved, this, [this](const QUuid &scriptId){
        QVariantMap params;
        params.insert("id", scriptId);
        emit ScriptAdded(params);
    });
    connect(m_engine, &ScriptEngine::scriptChanged, this, [this](const Script &script){
        QVariantMap params;
        params.insert("script", pack(script));
        emit ScriptChanged(params);
    });
    connect(m_engine, &ScriptEngine::scriptConsoleMessage, this, [this](const QUuid &scriptId, ScriptEngine::ScriptMessageType type, const QString &message){
        QVariantMap params;
        params.insert("scriptId", scriptId);
        params.insert("type", enumValueName(type));
        params.insert("message", message);
        emit ScriptLogMessage(params);
    });
}

QString ScriptsHandler::name() const
{
    return "Scripts";
}

JsonReply *ScriptsHandler::GetScripts(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantMap returns;
    returns.insert("scripts", pack(m_engine->scripts()));
    return createReply(returns);
}

JsonReply *ScriptsHandler::GetScriptContent(const QVariantMap &params)
{
    QUuid scriptId = params.value("id").toUuid();
    ScriptEngine::GetScriptReply reply = m_engine->scriptContent(scriptId);
    QVariantMap returns;
    returns.insert("scriptError", enumValueName(reply.scriptError));
    if (reply.scriptError == ScriptEngine::ScriptErrorNoError) {
        returns.insert("content", reply.content);
    }
    return createReply(returns);
}

JsonReply* ScriptsHandler::AddScript(const QVariantMap &params)
{
    qWarning() << "Script:" << params.value("content").toString();
    QVariantMap returns;

    ScriptEngine::AddScriptReply scriptReply = m_engine->addScript(params.value("name").toString(), params.value("content").toByteArray());

    returns.insert("scriptError", enumValueName(scriptReply.scriptError));
    if (scriptReply.scriptError != ScriptEngine::ScriptErrorNoError) {
        returns.insert("errors", scriptReply.errors);
    } else {
        returns.insert("script", pack(scriptReply.script));
    }
    return createReply(returns);
}

JsonReply *ScriptsHandler::EditScript(const QVariantMap &params)
{
    QUuid scriptId = params.value("id").toUuid();
    QByteArray content = params.value("content").toByteArray();

    QVariantMap returns;

    ScriptEngine::EditScriptReply reply = m_engine->editScript(scriptId, content);
    returns.insert("scriptError", enumValueName(reply.scriptError));
    if (reply.scriptError != ScriptEngine::ScriptErrorNoError) {
        returns.insert("errors", reply.errors);
    }
    return createReply(returns);
}

JsonReply *ScriptsHandler::RemoveScript(const QVariantMap &params)
{
    QUuid scriptId = params.value("id").toUuid();
    ScriptEngine::ScriptError status = m_engine->removeScript(scriptId);
    QVariantMap returns;
    returns.insert("scriptError", enumValueName(status));
    return createReply(returns);

}

}
