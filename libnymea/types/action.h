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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ACTION_H
#define ACTION_H

#include "libnymea.h"
#include "typeutils.h"
#include "param.h"

#include <QVariantList>

class LIBNYMEA_EXPORT Action
{
    Q_GADGET
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId WRITE setActionTypeId)
    Q_PROPERTY(QUuid thingId READ thingId WRITE setThingId)
    Q_PROPERTY(ParamList params READ params WRITE setParams USER true)

public:
    enum TriggeredBy {
        TriggeredByUser,
        TriggeredByRule,
        TriggeredByScript
    };
    explicit Action(const ActionTypeId &actionTypeId = ActionTypeId(), const ThingId &thingId = ThingId(), TriggeredBy triggeredBy = TriggeredByUser);
    Action(const Action &other);

    bool isValid() const;

    ActionTypeId actionTypeId() const;
    void setActionTypeId(const ActionTypeId &actionTypeId);
    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    ParamList params() const;
    void setParams(const ParamList &params);
    Param param(const ParamTypeId &paramTypeId) const;
    QVariant paramValue(const ParamTypeId &paramTypeId) const;

    TriggeredBy triggeredBy() const;

    void operator=(const Action &other);
private:
    ActionTypeId m_actionTypeId;
    ThingId m_thingId;
    ParamList m_params;
    TriggeredBy m_triggeredBy = TriggeredByUser;
};

#endif // ACTION_H
