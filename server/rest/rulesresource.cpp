/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "rulesresource.h"
#include "httprequest.h"
#include "typeutils.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QJsonDocument>

namespace guhserver {

RulesResource::RulesResource(QObject *parent) :
    RestResource(parent)
{
}

QString RulesResource::name() const
{
    return "rules";
}

HttpReply *RulesResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // /api/v1/rules/{ruleId}/
    if (urlTokens.count() >= 4) {
        m_ruleId = RuleId(urlTokens.at(3));
        if (m_ruleId.isNull()) {
            qCWarning(dcRest) << "Could not parse RuleId:" << urlTokens.at(3);
            return createErrorReply(HttpReply::BadRequest);
        }

        if (GuhCore::instance()->findRule(m_ruleId).id().isNull())
            return createErrorReply(HttpReply::NotFound);

    }

    // check method
    HttpReply *reply;
    switch (request.method()) {
    case HttpRequest::Get:
        reply = proccessGetRequest(request, urlTokens);
        break;
    case HttpRequest::Put:
        reply = proccessPutRequest(request, urlTokens);
        break;
    case HttpRequest::Post:
        reply = proccessPostRequest(request, urlTokens);
        break;
    case HttpRequest::Delete:
        reply = proccessDeleteRequest(request, urlTokens);
        break;
    default:
        reply = createErrorReply(HttpReply::BadRequest);
        break;
    }
    return reply;
}

HttpReply *RulesResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)

    // GET /api/v1/rules
    if (urlTokens.count() == 3) {
        // check if we should filter for rules containing a certain device
        DeviceId deviceId;
        if (request.url().hasQuery() && request.urlQuery().hasQueryItem("deviceId")) {
            deviceId = DeviceId(request.urlQuery().queryItemValue("deviceId"));
            if (deviceId.isNull())
                createErrorReply(HttpReply::BadRequest);
        }
        return getRules(deviceId);
    }

    // GET /api/v1/rules/{ruleId}
    if (urlTokens.count() == 4)
        return getRuleDetails(m_ruleId);

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)

    // DELETE /api/v1/rules
    if (urlTokens.count() == 3)
        return createErrorReply(HttpReply::BadRequest);

    // DELETE /api/v1/rules/{ruleId}
    if (urlTokens.count() == 4)
        return removeRule(m_ruleId);

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // PUT /api/v1/rules
    if (urlTokens.count() == 3)
        return createErrorReply(HttpReply::BadRequest);

    // PUT /api/v1/rules/{ruleId}
    if (urlTokens.count() == 4)
        return editRule(m_ruleId, request.payload());

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens)
{    
    // POST /api/v1/rules
    if (urlTokens.count() == 3)
        return addRule(request.payload());

    // POST /api/v1/rules/{ruleId}/enable
    if (urlTokens.count() == 5 && urlTokens.at(4) == "enable")
        return enableRule(m_ruleId);

    // POST /api/v1/rules/{ruleId}/disable
    if (urlTokens.count() == 5 && urlTokens.at(4) == "disable")
        return disableRule(m_ruleId);

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::getRules(const DeviceId &deviceId) const
{
    HttpReply *reply = createSuccessReply();

    if (deviceId.isNull()) {
        qCDebug(dcRest) << "Get rule descriptions";
        reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRuleDescriptions()).toJson());
    } else {
        qCDebug(dcRest) << "Get rule descriptions which contain the device with id" << deviceId.toString();
        QList<RuleId> ruleIdsList = GuhCore::instance()->findRules(deviceId);
        QList<Rule> ruleList;
        foreach (const RuleId &ruleId, ruleIdsList) {
            Rule rule = GuhCore::instance()->findRule(ruleId);
            if (!rule.id().isNull())
                ruleList.append(rule);
        }
        reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRuleDescriptions(ruleList)).toJson());
    }
    return reply;
}

HttpReply *RulesResource::getRuleDetails(const RuleId &ruleId) const
{
    Rule rule = GuhCore::instance()->findRule(ruleId);
    if (rule.id().isNull())
        return createErrorReply(HttpReply::NotFound);

    qCDebug(dcRest) << "Get rule details";
    HttpReply *reply = createSuccessReply();
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRule(rule)).toJson());
    return reply;
}

HttpReply *RulesResource::removeRule(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Remove rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->removeRule(ruleId);

    if (status == RuleEngine::RuleErrorNoError)
        return createSuccessReply();

    return createErrorReply(HttpReply::InternalServerError);
}

HttpReply *RulesResource::addRule(const QByteArray &payload) const
{
    qCDebug(dcRest) << "Add new rule";

    QPair<bool, QVariant> verification = RestResource::verifyPayload(payload);
    if (!verification.first)
        return createErrorReply(HttpReply::BadRequest);

    QVariantMap params = verification.second.toMap();

    // check rule consistency
    RuleEngine::RuleError ruleConsistencyError = JsonTypes::verifyRuleConsistency(params);
    if (ruleConsistencyError !=  RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = JsonTypes::verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    // Check and unpack stateEvaluator
    qCDebug(dcRest) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = JsonTypes::verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);


    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = JsonTypes::verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();

    RuleId newRuleId = RuleId::createRuleId();
    RuleEngine::RuleError status = GuhCore::instance()->addRule(newRuleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled);

    if (status ==  RuleEngine::RuleErrorNoError) {
        QVariantMap returns;
        returns.insert("id", newRuleId.toString());
        HttpReply *reply = createSuccessReply();
        reply->setPayload(QJsonDocument::fromVariant(returns).toJson());
        return reply;
    }

    return createErrorReply(HttpReply::BadRequest);
}

HttpReply *RulesResource::enableRule(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Enable rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->enableRule(ruleId);
    if (status ==  RuleEngine::RuleErrorNoError)
        return createSuccessReply();

    return createErrorReply(HttpReply::BadRequest);
}

HttpReply *RulesResource::disableRule(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Disable rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->disableRule(ruleId);
    if (status ==  RuleEngine::RuleErrorNoError)
        return createSuccessReply();

    return createErrorReply(HttpReply::BadRequest);
}

HttpReply *RulesResource::editRule(const RuleId &ruleId, const QByteArray &payload) const
{
    qCDebug(dcRest) << "Edit rule with id" << ruleId.toString();

    QPair<bool, QVariant> verification = RestResource::verifyPayload(payload);
    if (!verification.first)
        return createErrorReply(HttpReply::BadRequest);

    QVariantMap params = verification.second.toMap();

    // check rule consistency
    RuleEngine::RuleError ruleConsistencyError = JsonTypes::verifyRuleConsistency(params);
    if (ruleConsistencyError !=  RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = JsonTypes::verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    // Check and unpack stateEvaluator
    qCDebug(dcRest) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = JsonTypes::verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = JsonTypes::verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError)
        return createErrorReply(HttpReply::BadRequest);

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();

    RuleEngine::RuleError status = GuhCore::instance()->editRule(ruleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled);

    if (status ==  RuleEngine::RuleErrorNoError) {
        qCDebug(dcRest) << "Edit rule successfully finished";
        return createSuccessReply();
    }

    qCWarning(dcRest) << "Edit rule finished with error" << status;
    return createErrorReply(HttpReply::BadRequest);
}

}

