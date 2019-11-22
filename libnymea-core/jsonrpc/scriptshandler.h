#ifndef SCRIPTSHANDLER_H
#define SCRIPTSHANDLER_H

#include "jsonrpc/jsonhandler.h"

#include "scriptengine/scriptengine.h"

namespace nymeaserver {


class ScriptsHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ScriptsHandler(ScriptEngine *scriptEngine, QObject *parent = nullptr);

    QString name() const override;

public slots:
    JsonReply* GetScripts(const QVariantMap &params);
    JsonReply* GetScriptContent(const QVariantMap &params);
    JsonReply* AddScript(const QVariantMap &params);
    JsonReply* EditScript(const QVariantMap &params);
    JsonReply* RemoveScript(const QVariantMap &params);

signals:
    void ScriptAdded(const QVariantMap &params);
    void ScriptRemoved(const QVariantMap &params);
    void ScriptChanged(const QVariantMap &params);
    void ScriptLogMessage(const QVariantMap &params);

private:
    ScriptEngine *m_engine = nullptr;
};

}

#endif // SCRIPTSHANDLER_H
