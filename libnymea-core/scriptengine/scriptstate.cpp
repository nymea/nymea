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

#include "scriptstate.h"

#include "loggingcategories.h"

#include <QColor>
#include <qqml.h>
#include <QQmlEngine>

namespace nymeaserver {

ScriptState::ScriptState(QObject *parent) : QObject(parent)
{

}

void ScriptState::classBegin()
{
    m_deviceManager = reinterpret_cast<DeviceManager*>(qmlEngine(this)->property("deviceManager").toULongLong());
    connect(m_deviceManager, &DeviceManager::deviceStateChanged, this, &ScriptState::onDeviceStateChanged);
}

void ScriptState::componentComplete()
{

}

QString ScriptState::deviceId() const
{
    return m_deviceId;
}

void ScriptState::setDeviceId(const QString &deviceId)
{
    if (m_deviceId != deviceId) {
        m_deviceId = deviceId;
        emit deviceIdChanged();
        store();
    }
}

QString ScriptState::stateTypeId() const
{
    return m_stateTypeId;
}

void ScriptState::setStateTypeId(const QString &stateTypeId)
{
    if (m_stateTypeId != stateTypeId) {
        m_stateTypeId = stateTypeId;
        emit stateTypeChanged();
        store();
    }
}

QString ScriptState::stateName() const
{
    return m_stateName;
}

void ScriptState::setStateName(const QString &stateName)
{
    if (m_stateName != stateName) {
        m_stateName = stateName;
        emit stateTypeChanged();
        store();
    }
}

QVariant ScriptState::value() const
{
    Device* device = m_deviceManager->findConfiguredDevice(DeviceId(m_deviceId));
    if (!device) {
        return QVariant();
    }
    StateTypeId stateTypeId = StateTypeId(m_stateTypeId);
    if (stateTypeId.isNull()) {
        stateTypeId = device->deviceClass().stateTypes().findByName(m_stateName).id();
    }

    return device->stateValue(stateTypeId);
}

void ScriptState::setValue(const QVariant &value)
{
    qCDebug(dcScriptEngine()) << "setValueCalled1" << value;
    if (m_pendingActionInfo) {
        m_valueCache = value;
        return;
    }

    Device* device = m_deviceManager->findConfiguredDevice(DeviceId(m_deviceId));
    if (!device) {
        qCWarning(dcScriptEngine()) << "No device with id" << m_deviceId << "found.";
        return;
    }

    ActionTypeId actionTypeId;
    if (!m_stateTypeId.isNull()) {
        actionTypeId = device->deviceClass().stateTypes().findById(StateTypeId(m_stateTypeId)).id();
        if (actionTypeId.isNull()) {
            qCWarning(dcScriptEngine) << "Device" << device->name() << "does not have a state with type id" << m_stateTypeId;
        }
    }
    if (actionTypeId.isNull()) {
        actionTypeId = device->deviceClass().stateTypes().findByName(stateName()).id();
        if (actionTypeId.isNull()) {
            qCWarning(dcScriptEngine) << "Device" << device->name() << "does not have a state named" << m_stateName;
        }
    }

    if (actionTypeId.isNull()) {
        qCWarning(dcScriptEngine()) << "Either stateTypeId or stateName is required to be valid.";
        return;
    }

    Action action;
    action.setDeviceId(DeviceId(m_deviceId));
    action.setActionTypeId(ActionTypeId(actionTypeId));
    ParamList params = ParamList() << Param(ParamTypeId(actionTypeId), value);
    action.setParams(params);

    m_valueCache = QVariant();
    m_pendingActionInfo = m_deviceManager->executeAction(action);
    connect(m_pendingActionInfo, &DeviceActionInfo::finished, this, [this](){
        m_pendingActionInfo = nullptr;
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }
    });
}

QVariant ScriptState::minimumValue() const
{
    Device *device = m_deviceManager->configuredDevices().findById(DeviceId(m_deviceId));
    if (!device) {
        return QVariant();
    }
    StateType stateType = device->deviceClass().stateTypes().findById(StateTypeId(m_stateTypeId));
    if (stateType.id().isNull()) {
        stateType = device->deviceClass().stateTypes().findByName(m_stateName);
    }
    return stateType.minValue();
}

QVariant ScriptState::maximumValue() const
{
    Device *device = m_deviceManager->configuredDevices().findById(DeviceId(m_deviceId));
    if (!device) {
        return QVariant();
    }
    StateType stateType = device->deviceClass().stateTypes().findById(StateTypeId(m_stateTypeId));
    if (stateType.id().isNull()) {
        stateType = device->deviceClass().stateTypes().findByName(m_stateName);
    }
    return stateType.minValue();
}

void ScriptState::store()
{
    m_valueStore = value();
}

void ScriptState::restore()
{
    setValue(m_valueStore);
}

void nymeaserver::ScriptState::onDeviceStateChanged(Device *device, const StateTypeId &stateTypeId)
{
    if (device->id() != DeviceId(m_deviceId)) {
        return;
    }
    StateTypeId localStateTypeId = StateTypeId(m_stateTypeId);
    if (localStateTypeId.isNull()) {
        localStateTypeId = device->deviceClass().stateTypes().findByName(m_stateName).id();
    }
    if (localStateTypeId.isNull()) {
        return;
    }
    if (stateTypeId == localStateTypeId) {
        emit valueChanged();
    }
}

}
