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

#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include "types/param.h"
#include "typeutils.h"

#include <QDateTime>
#include <QTcpServer>
#include <QUuid>

class Thing;
class IntegrationPlugin;

class HttpDaemon : public QTcpServer
{
    Q_OBJECT
public:
    HttpDaemon(Thing *thing, IntegrationPlugin *parent = nullptr);
    ~HttpDaemon();
    void incomingConnection(qintptr socket) override;

    void actionExecuted(const ActionTypeId &actionTypeId);

signals:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &eventTypeId, const ParamList &params);
    void disappear();
    void reconfigureAutodevice();

private slots:
    void readClient();
    void discardClient();

private:
    QString generateHeader();
    QString generateWebPage();

private:
    bool disabled;

    IntegrationPlugin *m_plugin;
    Thing *m_thing;

    QList<QPair<ActionTypeId, QDateTime> > m_actionList;
};

#endif // HTTPDAEMON_H
