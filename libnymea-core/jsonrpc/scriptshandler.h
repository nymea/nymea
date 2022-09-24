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
    JsonReply* GetScripts(const QVariantMap &params);
    JsonReply* GetScriptContent(const QVariantMap &params);
    JsonReply* AddScript(const QVariantMap &params);
    JsonReply* EditScript(const QVariantMap &params);
    JsonReply* RemoveScript(const QVariantMap &params);

signals:
    void ScriptAdded(const QVariantMap &params);
    void ScriptRemoved(const QVariantMap &params);
    void ScriptChanged(const QVariantMap &params);
    void ScriptContentChanged(const QVariantMap &params);
    void ScriptLogMessage(const QVariantMap &params);

private:
    scriptengine::ScriptEngine *m_engine = nullptr;
};

}

#endif // SCRIPTSHANDLER_H
