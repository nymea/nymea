// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "testhelper.h"

TestHelper *TestHelper::s_instance = nullptr;

TestHelper *TestHelper::instance()
{
    if (!s_instance) {
        s_instance = new TestHelper();
    }
    return s_instance;
}

void TestHelper::logEvent(const QString &thingId, const QString &eventId, const QVariantMap &params)
{
    emit eventLogged(ThingId(thingId), eventId, params);
}

void TestHelper::logStateChange(const QString &thingId, const QString &stateId, const QVariant &value)
{
    emit stateChangeLogged(ThingId(thingId), stateId, value);
}

void TestHelper::logActionExecuted(const QString &thingId, const QString &actionId, const QVariantMap &params, Thing::ThingError status, Action::TriggeredBy triggeredBy)
{
    emit actionExecutionLogged(ThingId(thingId), actionId, params, status, triggeredBy);
}

void TestHelper::setTestResult(bool success)
{
    emit testResult(success);
}

TestHelper::TestHelper(QObject *parent)
    : QObject(parent)
{}
