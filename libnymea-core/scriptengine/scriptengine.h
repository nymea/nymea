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

    struct AddScriptReply {
        ScriptError scriptError;
        QStringList errors;
        Script script;
    };
    struct EditScriptReply {
        ScriptError scriptError;
        QStringList errors;
    };

    explicit ScriptEngine(DeviceManager *deviceManager, QObject *parent = nullptr);

    Scripts scripts();
    AddScriptReply addScript(const QString &name, const QByteArray &content);
    EditScriptReply editScript(const QUuid &id, const QByteArray &content);
    ScriptError removeScript(const QUuid &id);

private:
    void loadScripts();
    bool loadScript(Script *script);
    void unloadScript(Script *script);

private:
    QString baseName(const QUuid &id);

private:
    DeviceManager *m_deviceManager = nullptr;
    QQmlEngine *m_engine = nullptr;

    QHash<QUuid, Script*> m_scripts;
};

}

#endif // SCRIPTENGINE_H
