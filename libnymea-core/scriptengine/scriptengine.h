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

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QUuid>
#include <QQmlEngine>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QMutex>

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

    explicit ScriptEngine(ThingManager *thingManager, LogEngine *logEngine, QObject *parent = nullptr);
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
    ThingManager *m_thingManager = nullptr;
    QQmlEngine *m_engine = nullptr;
    Logger *m_logger = nullptr;

    QHash<QUuid, Script*> m_scripts;

    static QList<ScriptEngine*> s_engines;
    static QtMessageHandler s_upstreamMessageHandler;
    static QLoggingCategory::CategoryFilter s_oldCategoryFilter;
    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
    static void logCategoryFilter(QLoggingCategory *category);
    static QMutex s_loggerMutex;
};

}
}

#endif // SCRIPTENGINE_H
