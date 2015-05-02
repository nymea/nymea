/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICE_H
#define DEVICE_H

#include "typeutils.h"

#include "plugin/deviceclass.h"
#include "types/state.h"
#include "types/param.h"

#include <QObject>
#include <QUuid>
#include <QVariant>


class Device: public QObject
{
    Q_OBJECT

    friend class DeviceManager;

public:
    DeviceId id() const;
    DeviceClassId deviceClassId() const;
    PluginId pluginId() const;

    QString name() const;
    void setName(const QString &name);

    ParamList params() const;
    bool hasParam(const QString &paramName) const;
    void setParams(const ParamList &params);

    QVariant paramValue(const QString &paramName) const;
    void setParamValue(const QString &paramName, const QVariant &value);

    QList<State> states() const;
    bool hasState(const StateTypeId &stateTypeId) const;
    void setStates(const QList<State> &states);

    QVariant stateValue(const StateTypeId &stateTypeId) const;
    void setStateValue(const StateTypeId &stateTypeId, const QVariant &value);

    State state(const StateTypeId &stateTypeId) const;

    bool setupComplete() const;

signals:
    void stateValueChanged(const QUuid &stateTypeId, const QVariant &value);

private:
    Device(const PluginId &pluginId, const DeviceId &id, const DeviceClassId &deviceClassId, QObject *parent = 0);
    Device(const PluginId &pluginId, const DeviceClassId &deviceClassId, QObject *parent = 0);

    void setupCompleted();

private:
    DeviceId m_id;
    DeviceClassId m_deviceClassId;
    PluginId m_pluginId;
    QString m_name;
    ParamList m_params;
    QList<State> m_states;
    bool m_setupComplete;
};

#endif
