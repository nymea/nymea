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

/*!
    \class guhserver::RulesResource
    \brief This subclass of \l{RestResource} processes the REST requests for the \tt Rules namespace.

    \ingroup json
    \inmodule core

    This \l{RestResource} will be created in the \l{RestServer} and used to handle REST requests
    for the \tt {Rules} namespace of the API.

    \code
        http://localhost:3333/api/v1/rules
    \endcode

    \sa Rule, RestResource, RestServer
*/

#include "rulesresource.h"
#include "httprequest.h"
#include "typeutils.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QJsonDocument>

namespace guhserver {

/*! Constructs a \l RulesResource with the given \a parent. */
RulesResource::RulesResource(QObject *parent) :
    RestResource(parent)
{
}

/*! Returns the name of the \l{RestResource}. In this case \b rules.

    \sa RestResource::name()
*/
QString RulesResource::name() const
{
    return "rules";
}

/*! This method will be used to process the given \a request and the given \a urlTokens. The request
    has to be in this namespace. Returns the resulting \l HttpReply.

    \sa HttpRequest, HttpReply, RestResource::proccessRequest()
*/
HttpReply *RulesResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // /api/v1/rules/{ruleId}/
    if (urlTokens.count() >= 4) {
        m_ruleId = RuleId(urlTokens.at(3));
        if (m_ruleId.isNull()) {
            qCWarning(dcRest) << "Could not parse RuleId:" << urlTokens.at(3);
            return createRuleErrorReply(HttpReply::BadRequest, RuleEngine::RuleErrorRuleNotFound);
        }

        if (GuhCore::instance()->ruleEngine()->findRule(m_ruleId).id().isNull())
            return createRuleErrorReply(HttpReply::NotFound, RuleEngine::RuleErrorRuleNotFound);

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
    case HttpRequest::Options:
        reply = proccessOptionsRequest(request, urlTokens);
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
                return createRuleErrorReply(HttpReply::NotFound, RuleEngine::RuleErrorDeviceNotFound);
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

    // POST /api/v1/rules/{ruleId}/executeactions
    if (urlTokens.count() == 5 && urlTokens.at(4) == "executeactions")
        return executeActions(m_ruleId);

    // POST /api/v1/rules/{ruleId}/executeexitactions
    if (urlTokens.count() == 5 && urlTokens.at(4) == "executeexitactions")
        return executeExitActions(m_ruleId);

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessOptionsRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return RestResource::createCorsSuccessReply();
}

HttpReply *RulesResource::getRules(const DeviceId &deviceId) const
{
    HttpReply *reply = createSuccessReply();

    if (deviceId.isNull()) {
        qCDebug(dcRest) << "Get rule descriptions";
        reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRuleDescriptions()).toJson());
    } else {
        qCDebug(dcRest) << "Get rule descriptions which contain the device with id" << deviceId.toString();
        QList<RuleId> ruleIdsList = GuhCore::instance()->ruleEngine()->findRules(deviceId);
        QList<Rule> ruleList;
        foreach (const RuleId &ruleId, ruleIdsList) {
            Rule rule = GuhCore::instance()->ruleEngine()->findRule(ruleId);
            if (!rule.id().isNull())
                ruleList.append(rule);
        }
        reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
        reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRuleDescriptions(ruleList)).toJson());
    }
    return reply;
}

HttpReply *RulesResource::getRuleDetails(const RuleId &ruleId) const
{
    Rule rule = GuhCore::instance()->ruleEngine()->findRule(ruleId);
    if (rule.id().isNull())
        return createRuleErrorReply(HttpReply::NotFound, RuleEngine::RuleErrorRuleNotFound);

    qCDebug(dcRest) << "Get rule details";
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRule(rule)).toJson());
    return reply;
}

HttpReply *RulesResource::removeRule(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Remove rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->removeRule(ruleId);

    if (status != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::InternalServerError, status);

    return createRuleErrorReply(HttpReply::Ok, status);
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
        return createRuleErrorReply(HttpReply::BadRequest, ruleConsistencyError);

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = JsonTypes::verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::BadRequest, eventDescriptorVerification.second);

    // Check and unpack stateEvaluator
    qCDebug(dcRest) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());
    if (!stateEvaluator.isValid())
        return createRuleErrorReply(HttpReply::BadRequest, RuleEngine::RuleErrorInvalidStateEvaluatorValue);

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = JsonTypes::verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::BadRequest, actionsVerification.second);

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = JsonTypes::verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::BadRequest, exitActionsVerification.second);

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();
    bool executable = params.value("executable", true).toBool();

    RuleId newRuleId = RuleId::createRuleId();
    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->addRule(newRuleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled, executable);

    if (status ==  RuleEngine::RuleErrorNoError) {
        QVariant returns = JsonTypes::packRule(GuhCore::instance()->ruleEngine()->findRule(newRuleId));
        HttpReply *reply = createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
        reply->setPayload(QJsonDocument::fromVariant(returns).toJson());
        return reply;
    }

    return createRuleErrorReply(HttpReply::BadRequest, status);
}

HttpReply *RulesResource::enableRule(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Enable rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->enableRule(ruleId);

    if (status != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::InternalServerError, status);

    return createRuleErrorReply(HttpReply::Ok, status);
}

HttpReply *RulesResource::disableRule(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Disable rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->disableRule(ruleId);

    if (status != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::InternalServerError, status);

    return createRuleErrorReply(HttpReply::Ok, status);
}

HttpReply *RulesResource::executeActions(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Execute actions of rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->executeActions(ruleId);

    if (status != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::InternalServerError, status);

    return createRuleErrorReply(HttpReply::Ok, status);
}

HttpReply *RulesResource::executeExitActions(const RuleId &ruleId) const
{
    qCDebug(dcRest) << "Execute exit actions of rule with id" << ruleId.toString();

    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->executeExitActions(ruleId);

    if (status != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::InternalServerError, status);

    return createRuleErrorReply(HttpReply::Ok, status);
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
        return createRuleErrorReply(HttpReply::BadRequest, ruleConsistencyError);

    // Check and upack eventDescriptorList
    QPair<QList<EventDescriptor>, RuleEngine::RuleError> eventDescriptorVerification = JsonTypes::verifyEventDescriptors(params);
    QList<EventDescriptor> eventDescriptorList = eventDescriptorVerification.first;
    if (eventDescriptorVerification.second != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::BadRequest, eventDescriptorVerification.second);

    // Check and unpack stateEvaluator
    qCDebug(dcRest) << "unpacking stateEvaluator:" << params.value("stateEvaluator").toMap();
    StateEvaluator stateEvaluator = JsonTypes::unpackStateEvaluator(params.value("stateEvaluator").toMap());
    if (!stateEvaluator.isValid())
        return createRuleErrorReply(HttpReply::BadRequest, RuleEngine::RuleErrorInvalidStateEvaluatorValue);

    // Check and unpack actions
    QPair<QList<RuleAction>, RuleEngine::RuleError> actionsVerification = JsonTypes::verifyActions(params, eventDescriptorList);
    QList<RuleAction> actions = actionsVerification.first;
    if (actionsVerification.second != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::BadRequest, actionsVerification.second);

    // Check and unpack exitActions
    QPair<QList<RuleAction>, RuleEngine::RuleError> exitActionsVerification = JsonTypes::verifyExitActions(params);
    QList<RuleAction> exitActions = exitActionsVerification.first;
    if (exitActionsVerification.second != RuleEngine::RuleErrorNoError)
        return createRuleErrorReply(HttpReply::BadRequest, exitActionsVerification.second);

    QString name = params.value("name", QString()).toString();
    bool enabled = params.value("enabled", true).toBool();

    RuleEngine::RuleError status = GuhCore::instance()->ruleEngine()->editRule(ruleId, name, eventDescriptorList, stateEvaluator, actions, exitActions, enabled);

    if (status ==  RuleEngine::RuleErrorNoError) {
        qCDebug(dcRest) << "Edit rule successfully finished";
        return createRuleErrorReply(HttpReply::Ok, status);
    }

    qCWarning(dcRest) << "Edit rule finished with error" << status;
    return createRuleErrorReply(HttpReply::BadRequest, status);
}

}

