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

#ifndef RULESRESOURCE_H
#define RULESRESOURCE_H

#include <QObject>
#include <QHash>

#include "jsontypes.h"
#include "restresource.h"
#include "httpreply.h"


namespace guhserver {

class HttpRequest;

class RulesResource : public RestResource
{
    Q_OBJECT
public:
    explicit RulesResource(QObject *parent = 0);

    QString name() const override;

    HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) override;

private:
    RuleId m_ruleId;

    // Process method
    HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens) override;
    HttpReply *proccessOptionsRequest(const HttpRequest &request, const QStringList &urlTokens) override;

    // Get methods
    HttpReply *getRules(const DeviceId &deviceId) const;
    HttpReply *getRuleDetails(const RuleId &ruleId) const;

    // Delete methods
    HttpReply *removeRule(const RuleId &ruleId) const;

    // Post methods
    HttpReply *addRule(const QByteArray &payload) const;
    HttpReply *enableRule(const RuleId &ruleId) const;
    HttpReply *disableRule(const RuleId &ruleId) const;
    HttpReply *executeActions(const RuleId &ruleId) const;
    HttpReply *executeExitActions(const RuleId &ruleId) const;


    // Put methods
    HttpReply *editRule(const RuleId &ruleId, const QByteArray &payload) const;

};

}

#endif // RULESRESOURCE_H
