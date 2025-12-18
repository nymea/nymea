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

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QJsonValue>
#include <QLoggingCategory>
#include <QMutex>
#include <QObject>
#include <QQmlEngine>
#include <QUuid>

#include "script.h"

class ThingManager;
class LogEngine;
class Logger;

namespace nymeaserver {
namespace scriptengine {

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    enum ScriptError { ScriptErrorNoError, ScriptErrorScriptNotFound, ScriptErrorInvalidScript, ScriptErrorHardwareFailure };
    Q_ENUM(ScriptError)

    enum ScriptMessageType { ScriptMessageTypeLog, ScriptMessageTypeWarning };
    Q_ENUM(ScriptMessageType)

    struct AddScriptReply
    {
        ScriptError scriptError;
        QStringList errors;
        Script script;
    };
    struct EditScriptReply
    {
        ScriptError scriptError;
        QStringList errors;
    };
    struct GetScriptReply
    {
        ScriptError scriptError;
        QByteArray content;
    };

    explicit ScriptEngine(ThingManager *thingManager, LogEngine *logEngine, QObject *parent = nullptr);
    ~ScriptEngine();

    Scripts scripts();
    GetScriptReply scriptContent(const QUuid &id);
    AddScriptReply addScript(const QString &name, const QByteArray &content);
    ScriptError renameScript(const QUuid &id, const QString &name);
    EditScriptReply editScript(const QUuid &id, const QByteArray &content);
    ScriptError removeScript(const QUuid &id);

    void onScriptMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);

signals:
    void scriptAdded(const Script &script);
    void scriptRemoved(const QUuid &id);
    void scriptChanged(const Script &script);
    void scriptRenamed(const Script &script);

    void scriptConsoleMessage(const QUuid &scriptId, ScriptMessageType type, const QString &message);

private:
    void loadScripts();
    bool loadScript(Script *script);
    void unloadScript(Script *script);

    QString baseName(const QUuid &id);

private:
    ThingManager *m_thingManager = nullptr;
    QQmlEngine *m_engine = nullptr;
    Logger *m_logger = nullptr;

    QHash<QUuid, Script *> m_scripts;

    static QList<ScriptEngine *> s_engines;
    static QtMessageHandler s_upstreamMessageHandler;
    static QLoggingCategory::CategoryFilter s_oldCategoryFilter;
    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
    static void logCategoryFilter(QLoggingCategory *category);
    static QMutex s_loggerMutex;
};

} // namespace scriptengine
} // namespace nymeaserver

#endif // SCRIPTENGINE_H
