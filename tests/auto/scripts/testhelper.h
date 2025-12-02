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

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>

#include "typeutils.h"
#include "integrations/thing.h"
#include "types/action.h"

class TestHelper : public QObject
{
    Q_OBJECT
public:
    static TestHelper* instance();

    Q_INVOKABLE void logEvent(const QString &thingId, const QString &eventId, const QVariantMap &params);
    Q_INVOKABLE void logStateChange(const QString &thingId, const QString &stateId, const QVariant &value);
    Q_INVOKABLE void logActionExecuted(const QString &thingId, const QString &actionId, const QVariantMap &params, Thing::ThingError status, Action::TriggeredBy triggeredBy);

    Q_INVOKABLE void setTestResult(bool success);

signals:
    void setState(const QVariant &value);
    void executeAction(const QVariantMap &params);

    void eventLogged(const ThingId &thingId, const QString &eventId, const QVariantMap &params);
    void stateChangeLogged(const ThingId &thingId, const QString &stateId, const QVariant &value);
    void actionExecutionLogged(const ThingId &thingId, const QString &actionId, const QVariantMap &params, Thing::ThingError status, Action::TriggeredBy triggeredBy);

    void testResult(bool success);

private:
    explicit TestHelper(QObject *parent = nullptr);
    static TestHelper* s_instance;
};

#endif // TESTHELPER_H
