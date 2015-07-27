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
#include "network/httprequest.h"
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
    if (urlTokens.count() == 3)
        return getRules();

    // GET /api/v1/rules/{ruleId}
    if (urlTokens.count() == 4)
        return getRuleDetails(m_ruleId);

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RulesResource::getRules() const
{
    qCDebug(dcRest) << "Get rule descriptions";
    HttpReply *reply = createSuccessReply();
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRuleDescriptions()).toJson());
    return reply;
}

Rule RulesResource::findRule(const RuleId &ruleId) const
{
    foreach (const Rule &rule, GuhCore::instance()->rules()) {
        if (rule.id() == ruleId) {
            return rule;
        }
    }
    return Rule();
}

HttpReply *RulesResource::getRuleDetails(const RuleId &ruleId) const
{
    Rule rule = findRule(ruleId);
    if (rule.id().isNull())
        return createErrorReply(HttpReply::NotFound);

    qCDebug(dcRest) << "Get rule details";
    HttpReply *reply = createSuccessReply();
    reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packRule(rule)).toJson());
    return reply;
}

}

