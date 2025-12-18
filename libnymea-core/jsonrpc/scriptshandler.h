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

#ifndef SCRIPTSHANDLER_H
#define SCRIPTSHANDLER_H

#include "jsonrpc/jsonhandler.h"

#include "scriptengine/scriptengine.h"

namespace nymeaserver {

class ScriptsHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ScriptsHandler(scriptengine::ScriptEngine *scriptEngine, QObject *parent = nullptr);

    QString name() const override;

public slots:
    JsonReply *GetScripts(const QVariantMap &params);
    JsonReply *GetScriptContent(const QVariantMap &params);
    JsonReply *AddScript(const QVariantMap &params);
    JsonReply *EditScript(const QVariantMap &params);
    JsonReply *RemoveScript(const QVariantMap &params);

signals:
    void ScriptAdded(const QVariantMap &params);
    void ScriptRemoved(const QVariantMap &params);
    void ScriptChanged(const QVariantMap &params);
    void ScriptContentChanged(const QVariantMap &params);
    void ScriptLogMessage(const QVariantMap &params);

private:
    scriptengine::ScriptEngine *m_engine = nullptr;
};

} // namespace nymeaserver

#endif // SCRIPTSHANDLER_H
