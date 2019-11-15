#include "scriptaction.h"

#include "devices/devicemanager.h"
#include "types/action.h"

#include <QQmlEngine>

namespace nymeaserver {

ScriptAction::ScriptAction(QObject *parent) : QObject(parent)
{

}

void ScriptAction::classBegin()
{
    m_deviceManager = reinterpret_cast<DeviceManager*>(qmlEngine(this)->property("deviceManager").toULongLong());
}

void ScriptAction::componentComplete()
{

}

QString ScriptAction::deviceId() const
{
    return m_deviceId;
}

void ScriptAction::setDeviceId(const QString &deviceId)
{
    if (m_deviceId != deviceId) {
        m_deviceId = deviceId;
        emit deviceIdChanged();
    }
}

QString ScriptAction::actionTypeId() const
{
    return m_actionTypeId;
}

void ScriptAction::setActionTypeId(const QString &actionTypeId)
{
    if (m_actionTypeId != actionTypeId) {
        m_actionTypeId = actionTypeId;
        emit actionTypeIdChanged();
    }
}

void ScriptAction::execute(const QVariantList &params)
{
    Action action;
    action.setActionTypeId(ActionTypeId(m_actionTypeId));
    action.setDeviceId(DeviceId(m_deviceId));
    ParamList paramList;
    foreach (const QVariant &p, params) {
        paramList << Param(ParamTypeId(p.toMap().value("paramTypeId").toUuid()), p.toMap().value("value"));
    }
    action.setParams(paramList);
    m_deviceManager->executeAction(action);
}

}
