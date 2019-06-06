/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICE_H
#define DEVICE_H

#include "typeutils.h"
#include "libnymea.h"

#include "types/deviceclass.h"
#include "types/state.h"
#include "types/param.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

class LIBNYMEA_EXPORT Device: public QObject
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
    bool hasParam(const ParamTypeId &paramTypeId) const;
    void setParams(const ParamList &params);

    QVariant paramValue(const ParamTypeId &paramTypeId) const;
    void setParamValue(const ParamTypeId &paramName, const QVariant &value);

    ParamList settings() const;
    bool hasSetting(const ParamTypeId &paramTypeId) const;
    void setSettings(const ParamList &settings);

    QVariant setting(const ParamTypeId &paramTypeId) const;
    void setSettingValue(const ParamTypeId &paramTypeId, const QVariant &value);

    QList<State> states() const;
    bool hasState(const StateTypeId &stateTypeId) const;
    void setStates(const QList<State> &states);

    QVariant stateValue(const StateTypeId &stateTypeId) const;
    void setStateValue(const StateTypeId &stateTypeId, const QVariant &value);

    State state(const StateTypeId &stateTypeId) const;

    DeviceId parentId() const;
    void setParentId(const DeviceId &parentId);

    bool setupComplete() const;
    bool autoCreated() const;

signals:
    void stateValueChanged(const StateTypeId &stateTypeId, const QVariant &value);
    void settingChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    void nameChanged();

private:
    Device(const PluginId &pluginId, const DeviceId &id, const DeviceClassId &deviceClassId, QObject *parent = nullptr);
    Device(const PluginId &pluginId, const DeviceClassId &deviceClassId, QObject *parent = nullptr);

    void setupCompleted();
    void setSetupComplete(const bool &complete);

private:
    DeviceId m_id;
    DeviceId m_parentId;
    DeviceClassId m_deviceClassId;
    PluginId m_pluginId;
    QString m_name;
    ParamList m_params;
    ParamList m_settings;
    QList<State> m_states;
    bool m_setupComplete = false;
    bool m_autoCreated = false;
};

QDebug operator<<(QDebug dbg, Device *device);

class LIBNYMEA_EXPORT Devices: public QList<Device*>
{
public:
    Devices() = default;
    Devices(const QList<Device *> &other);
    Device* findById(const DeviceId &id);
    Devices filterByParam(const ParamTypeId &paramTypeId, const QVariant &value = QVariant());
    Devices filterByDeviceClassId(const DeviceClassId &deviceClassId);
};

#endif
