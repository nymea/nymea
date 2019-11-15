#include "scriptstate.h"

#include "loggingcategories.h"

#include <QColor>

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
        emit stateTypeIdChanged();
        store();
    }
}

QVariant ScriptState::value() const
{
    Device* device = m_deviceManager->findConfiguredDevice(DeviceId(m_deviceId));
    if (!device) {
        return QVariant();
    }
    return device->stateValue(StateTypeId(m_stateTypeId));
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

    if (device->deviceClass().stateTypes().findById(StateTypeId(m_stateTypeId)).id().isNull()) {
        qCWarning(dcScriptEngine) << "Device" << device->name() << "does not have a state with type id" << m_stateTypeId;
        return;
    }
    Action action;
    action.setDeviceId(DeviceId(m_deviceId));
    action.setActionTypeId(ActionTypeId(m_stateTypeId));
    ParamList params = ParamList() << Param(ParamTypeId(m_stateTypeId), value);
    action.setParams(params);

    qCDebug(dcScriptEngine()) << "setValueCalled2" << value;
    m_valueCache = QVariant();
    m_pendingActionInfo = m_deviceManager->executeAction(action);
    connect(m_pendingActionInfo, &DeviceActionInfo::finished, this, [this](){
        m_pendingActionInfo = nullptr;
        if (!m_valueCache.isNull()) {
            setValue(m_valueCache);
        }
    });
}

void ScriptState::store()
{
    m_valueStore = value();
    qCDebug(dcScriptEngine()) << "Storing value:" << m_valueStore;
}

void ScriptState::restore()
{
    qCDebug(dcScriptEngine()) << "Restoring value:" << m_valueStore << m_valueStore.value<QColor>().toRgb();
    setValue(m_valueStore);
}

void nymeaserver::ScriptState::onDeviceStateChanged(Device *device, const StateTypeId &stateTypeId)
{
    if (device->id() == DeviceId(m_deviceId) && stateTypeId == StateTypeId(m_stateTypeId)) {
        emit valueChanged();
    }
}

}
