// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "scriptshandler.h"

#include "loggingcategories.h"

#include "scriptengine/scriptengine.h"

namespace nymeaserver {

using namespace scriptengine;

ScriptsHandler::ScriptsHandler(ScriptEngine *scriptEngine, QObject *parent)
    : JsonHandler(parent)
    , m_engine(scriptEngine)
{
    registerEnum<ScriptEngine::ScriptError>();
    registerEnum<ScriptEngine::ScriptMessageType>();

    registerObject<scriptengine::Script, Scripts>();

    QVariantMap params, returns;
    QString description;

    params.clear();
    returns.clear();
    description = "Get all script, that is, their names and properties, but no content.";
    returns.insert("scripts", objectRef<Scripts>());
    registerMethod("GetScripts", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get a scripts content.";
    params.insert("id", enumValueName(Uuid));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    returns.insert("o:content", enumValueName(String));
    registerMethod("GetScriptContent", description, params, returns);

    params.clear();
    returns.clear();
    description = "Add a script";
    params.insert("name", enumValueName(String));
    params.insert("content", enumValueName(String));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    returns.insert("o:script", objectRef<Script>());
    returns.insert("o:errors", enumValueName(StringList));
    registerMethod("AddScript", description, params, returns);

    params.clear();
    returns.clear();
    description = "Edit a script";
    params.insert("id", enumValueName(Uuid));
    params.insert("o:name", enumValueName(String));
    params.insert("o:content", enumValueName(String));
    returns.insert("scriptError", enumRef<ScriptEngine::ScriptError>());
    returns.insert("o:errors", enumValueName(StringList));
    registerMethod("EditScript", description, params, returns);

    params.clear();
    returns.clear();
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
    description = "Emitted when a script has been changed in the system (e.g. renamed).";
    params.insert("scriptId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    registerNotification("ScriptChanged", description, params);

    params.clear();
    description = "Emitted when a script's content has been changed in the system.";
    params.insert("scriptId", enumValueName(Uuid));
    registerNotification("ScriptContentChanged", description, params);

    params.clear();
    description = "Emitted when a script produces a console message.";
    params.insert("scriptId", enumValueName(Uuid));
    params.insert("type", enumRef<ScriptEngine::ScriptMessageType>());
    params.insert("message", enumValueName(String));
    registerNotification("ScriptLogMessage", description, params);

    connect(m_engine, &ScriptEngine::scriptAdded, this, [this](const Script &script) {
        QVariantMap params;
        params.insert("script", pack(script));
        emit ScriptAdded(params);
    });
    connect(m_engine, &ScriptEngine::scriptRemoved, this, [this](const QUuid &scriptId) {
        QVariantMap params;
        params.insert("id", scriptId);
        emit ScriptRemoved(params);
    });
    connect(m_engine, &ScriptEngine::scriptRenamed, this, [this](const Script &script) {
        QVariantMap params;
        params.insert("scriptId", script.id());
        params.insert("name", script.name());
        emit ScriptChanged(params);
    });
    connect(m_engine, &ScriptEngine::scriptChanged, this, [this](const Script &script) {
        QVariantMap params;
        params.insert("scriptId", script.id());
        emit ScriptContentChanged(params);
    });
    connect(m_engine, &ScriptEngine::scriptConsoleMessage, this, [this](const QUuid &scriptId, ScriptEngine::ScriptMessageType type, const QString &message) {
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

JsonReply *ScriptsHandler::AddScript(const QVariantMap &params)
{
    qCDebug(dcJsonRpc()) << "Script:" << params.value("content").toString();
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
    QVariantMap returns;

    if (params.contains("name")) {
        QString name = params.value("name").toString();
        ScriptEngine::ScriptError result = m_engine->renameScript(scriptId, name);
        if (result != ScriptEngine::ScriptErrorNoError) {
            returns.insert("scriptError", enumValueName(result));
            return createReply(returns);
        }
    }
    if (params.contains("content")) {
        QByteArray content = params.value("content").toByteArray();
        ScriptEngine::EditScriptReply reply = m_engine->editScript(scriptId, content);
        if (reply.scriptError != ScriptEngine::ScriptErrorNoError) {
            returns.insert("scriptError", enumValueName(reply.scriptError));
            returns.insert("errors", reply.errors);
            return createReply(returns);
        }
    }

    returns.insert("scriptError", enumValueName(ScriptEngine::ScriptErrorNoError));
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

} // namespace nymeaserver
