/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include "typeutils.h"
#include "types/vendor.h"
#include "types/eventtype.h"
#include "types/actiontype.h"
#include "types/statetype.h"
#include "types/paramtype.h"

#include <QList>
#include <QUuid>

class DeviceClass
{
public:
    enum CreateMethod {
        CreateMethodUser,
        CreateMethodAuto,
        CreateMethodDiscovery
    };
    enum SetupMethod {
        SetupMethodJustAdd,
        SetupMethodDisplayPin,
        SetupMethodEnterPin,
        SetupMethodPushButton
    };

    DeviceClass(const PluginId &pluginId = PluginId(), const VendorId &vendorId = VendorId(), const DeviceClassId &id = DeviceClassId());

    DeviceClassId id() const;
    VendorId vendorId() const;
    PluginId pluginId() const;
    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QList<StateType> states() const;
    void setStates(const QList<StateType> &stateTypes);

    QList<EventType> events() const;
    void setEvents(const QList<EventType> &eventTypes);

    QList<ActionType> actions() const;
    void setActions(const QList<ActionType> &actionTypes);

    QList<ParamType> params() const;
    void setParams(const QList<ParamType> &params);

    CreateMethod createMethod() const;
    void setCreateMethod(CreateMethod createMethod);
    SetupMethod setupMethod() const;
    void setSetupMethod(SetupMethod setupMethod);

    bool operator==(const DeviceClass &device) const;

private:
    DeviceClassId m_id;
    VendorId m_vendorId;
    PluginId m_pluginId;
    QString m_name;
    QList<StateType> m_states;
    QList<EventType> m_events;
    QList<ActionType> m_actions;
    QList<ParamType> m_params;
    CreateMethod m_createMethod;
    SetupMethod m_setupMethod;
};

#endif
