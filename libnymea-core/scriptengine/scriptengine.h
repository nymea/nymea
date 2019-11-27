/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QUuid>
#include <QQmlEngine>

#include "devices/devicemanager.h"
#include "script.h"

namespace nymeaserver {

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    enum ScriptError {
        ScriptErrorNoError,
        ScriptErrorScriptNotFound,
        ScriptErrorInvalidScript,
        ScriptErrorHardwareFailure
    };
    Q_ENUM(ScriptError)

    enum ScriptMessageType {
        ScriptMessageTypeLog,
        ScriptMessageTypeWarning
    };
    Q_ENUM(ScriptMessageType)

    struct AddScriptReply {
        ScriptError scriptError;
        QStringList errors;
        Script script;
    };
    struct EditScriptReply {
        ScriptError scriptError;
        QStringList errors;
    };
    struct GetScriptReply {
        ScriptError scriptError;
        QByteArray content;
    };

    explicit ScriptEngine(DeviceManager *deviceManager, QObject *parent = nullptr);
    ~ScriptEngine();

    Scripts scripts();
    GetScriptReply scriptContent(const QUuid &id);
    AddScriptReply addScript(const QString &name, const QByteArray &content);
    ScriptError renameScript(const QUuid &id, const QString &name);
    EditScriptReply editScript(const QUuid &id, const QByteArray &content);
    ScriptError removeScript(const QUuid &id);

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

    void onScriptMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);
private:
    DeviceManager *m_deviceManager = nullptr;
    QQmlEngine *m_engine = nullptr;

    QHash<QUuid, Script*> m_scripts;

    static QtMessageHandler s_upstreamMessageHandler;
    static QList<ScriptEngine*> s_engines;
    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

}

#endif // SCRIPTENGINE_H
