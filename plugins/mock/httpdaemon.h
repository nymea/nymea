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

#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include "typeutils.h"
#include "types/param.h"

#include <QTcpServer>
#include <QUuid>
#include <QDateTime>

class Thing;
class IntegrationPlugin;

class HttpDaemon : public QTcpServer
{
    Q_OBJECT
public:
    HttpDaemon(Thing *thing, IntegrationPlugin* parent = nullptr);
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
