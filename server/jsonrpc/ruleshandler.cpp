/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "ruleshandler.h"

#include "guhcore.h"
#include "ruleengine.h"

#include <QDebug>

RulesHandler::RulesHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetRules", "Get all configured rules");
    setParams("GetRules", params);
    QVariantList rules;
    rules.append(JsonTypes::ruleRef());
    returns.insert("rules", rules);
    setReturns("GetRules", returns);

    params.clear(); returns.clear();
    setDescription("AddRule", "Add a rule");
    params.insert("event", JsonTypes::eventRef());
    QVariantList actions;
    actions.append(JsonTypes::actionRef());
    params.insert("actions", actions);
    setParams("AddRule", params);
    returns.insert("success", "bool");
    returns.insert("errorMessage", "string");
    setReturns("AddRule", returns);

    params.clear(); returns.clear();
    setDescription("RemoveRule", "Remove a rule");
    params.insert("ruleId", "uuid");
    setParams("RemoveRule", params);
    setReturns("RemoveRule", returns);
}

QString RulesHandler::name() const
{
    return "Rules";
}

JsonReply* RulesHandler::GetRules(const QVariantMap &params)
{
    Q_UNUSED(params)

    QVariantList rulesList;
    foreach (const Rule &rule, GuhCore::instance()->ruleEngine()->rules()) {
        qDebug() << "got rule" << rule.id();
        QVariantMap ruleMap = JsonTypes::packRule(rule);
        rulesList.append(ruleMap);
    }
    QVariantMap returns;
    returns.insert("rules", rulesList);

    return createReply(returns);
}

JsonReply* RulesHandler::AddRule(const QVariantMap &params)
{
    QVariantMap eventMap = params.value("event").toMap();

    EventTypeId eventTypeId(eventMap.value("eventTypeId").toString());
    DeviceId eventDeviceId(eventMap.value("deviceId").toString());
    QVariantMap eventParams = eventMap.value("params").toMap();
    Event event(eventTypeId, eventDeviceId, eventParams);

    QList<Action> actions;
    QVariantList actionList = params.value("actions").toList();
    qDebug() << "got action list" << actionList.count();
    foreach (const QVariant &actionVariant, actionList) {
        QVariantMap actionMap = actionVariant.toMap();
        Action action(DeviceId(actionMap.value("deviceId").toString()), ActionTypeId(actionMap.value("actionTypeId").toString()));
        action.setParams(actionMap.value("params").toMap());
        actions.append(action);
    }

    QVariantMap returns;
    if (actions.count() == 0) {
        returns.insert("success", false);
        returns.insert("errorMessage", "Missing parameter: \"actions\".");
        return createReply(returns);
    }

    switch(GuhCore::instance()->ruleEngine()->addRule(event, actions)) {
    case RuleEngine::RuleErrorNoError:
        returns.insert("success", true);
        returns.insert("errorMessage", "");
        break;
    case RuleEngine::RuleErrorDeviceNotFound:
        returns.insert("success", false);
        returns.insert("errorMessage", "No such device.");
        break;
    case RuleEngine::RuleErrorEventTypeNotFound:
        returns.insert("success", false);
        returns.insert("errorMessage", "Device does not have such a event type.");
        break;
    default:
        returns.insert("success", false);
        returns.insert("errorMessage", "Unknown error");
    }
    return createReply(returns);
}

JsonReply* RulesHandler::RemoveRule(const QVariantMap &params)
{
    QVariantMap returns;
    QUuid ruleId = params.value("ruleId").toUuid();
    switch (GuhCore::instance()->ruleEngine()->removeRule(ruleId)) {
    case RuleEngine::RuleErrorNoError:
        returns.insert("success", true);
        break;
    case RuleEngine::RuleErrorRuleNotFound:
        returns.insert("success", false);
        returns.insert("errorMessage", "No such rule.");
        break;
    default:
        returns.insert("success", false);
        returns.insert("errorMessage", "Unknown error");
    }
    return createReply(returns);
}
