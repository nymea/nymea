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

#ifndef PUSHBUTTONAGENT_H
#define PUSHBUTTONAGENT_H

#include <QDBusConnection>
#include <QObject>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcPushButtonAgent)

class PushButtonAgent : public QObject
{
    Q_OBJECT
public:
    explicit PushButtonAgent(QObject *parent = nullptr);

    bool init(QDBusConnection::BusType busType = QDBusConnection::SystemBus);

signals:
    Q_SCRIPTABLE void PushButtonPressed();

public slots:
    void sendButtonPressed();
};

#endif // PUSHBUTTONAGENT_H
