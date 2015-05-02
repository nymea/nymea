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

#include "stateevaluator.h"
#include "guhcore.h"
#include "devicemanager.h"

StateEvaluator::StateEvaluator(const StateDescriptor &stateDescriptor):
    m_stateDescriptor(stateDescriptor),
    m_operatorType(Types::StateOperatorAnd)
{

}

StateEvaluator::StateEvaluator(QList<StateEvaluator> childEvaluators, Types::StateOperator stateOperator):
    m_stateDescriptor(),
    m_childEvaluators(childEvaluators),
    m_operatorType(stateOperator)
{
}

StateDescriptor StateEvaluator::stateDescriptor() const
{
    return m_stateDescriptor;
}

QList<StateEvaluator> StateEvaluator::childEvaluators() const
{
    return m_childEvaluators;
}

void StateEvaluator::setChildEvaluators(const QList<StateEvaluator> &stateEvaluators)
{
    m_childEvaluators = stateEvaluators;
}

void StateEvaluator::appendEvaluator(const StateEvaluator &stateEvaluator)
{
    m_childEvaluators.append(stateEvaluator);
}

Types::StateOperator StateEvaluator::operatorType() const
{
    return m_operatorType;
}

void StateEvaluator::setOperatorType(Types::StateOperator operatorType)
{
    m_operatorType = operatorType;
}

bool StateEvaluator::evaluate() const
{
    if (m_stateDescriptor.isValid()) {
        Device *device = GuhCore::instance()->findConfiguredDevice(m_stateDescriptor.deviceId());
        if (!device) {
            qWarning() << "Device not existing!";
            return false;
        }
        if (!device->hasState(m_stateDescriptor.stateTypeId())) {
            qWarning() << "Device found, but it does not appear to have such a state!";
            return false;
        }
        if (m_stateDescriptor != device->state(m_stateDescriptor.stateTypeId())) {
            // state not matching
            return false;
        }
    }

    if (m_operatorType == Types::StateOperatorOr) {
        foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
            if (stateEvaluator.evaluate()) {
                return true;
            }
        }
        return false;
    }

    foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
        if (!stateEvaluator.evaluate()) {
            return false;
        }
    }
    return true;
}

bool StateEvaluator::containsDevice(const DeviceId &deviceId) const
{
    if (m_stateDescriptor.deviceId() == deviceId) {
        return true;
    }
    foreach (const StateEvaluator &childEvaluator, m_childEvaluators) {
        if (childEvaluator.containsDevice(deviceId)) {
            return true;
        }
    }
    return false;
}

void StateEvaluator::removeDevice(const DeviceId &deviceId)
{
    if (m_stateDescriptor.deviceId() == deviceId) {
        m_stateDescriptor = StateDescriptor();
    }
    for (int i = 0; i < m_childEvaluators.count(); i++) {
        m_childEvaluators[i].removeDevice(deviceId);
    }
}

void StateEvaluator::dumpToSettings(QSettings &settings, const QString &groupName) const
{
    settings.beginGroup(groupName);

    settings.beginGroup("stateDescriptor");
    settings.setValue("stateTypeId", m_stateDescriptor.stateTypeId().toString());
    settings.setValue("deviceId", m_stateDescriptor.deviceId().toString());
    settings.setValue("value", m_stateDescriptor.stateValue());
    settings.setValue("operator", m_stateDescriptor.operatorType());
    settings.endGroup();

    settings.setValue("operator", m_operatorType);

    settings.beginGroup("childEvaluators");
    for (int i = 0; i < m_childEvaluators.count(); i++) {
        m_childEvaluators.at(i).dumpToSettings(settings, "stateEvaluator-" + QString::number(i));
    }
    settings.endGroup();

    settings.endGroup();
}

StateEvaluator StateEvaluator::loadFromSettings(QSettings &settings, const QString &groupName)
{
    settings.beginGroup(groupName);
    settings.beginGroup("stateDescriptor");
    StateTypeId stateTypeId(settings.value("stateTypeId").toString());
    DeviceId deviceId(settings.value("deviceId").toString());
    QVariant stateValue = settings.value("value");
    Types::ValueOperator valueOperator = (Types::ValueOperator)settings.value("operator").toInt();
    StateDescriptor stateDescriptor(stateTypeId, deviceId, stateValue, valueOperator);
    settings.endGroup();

    StateEvaluator ret(stateDescriptor);
    ret.setOperatorType((Types::StateOperator)settings.value("operator").toInt());

    settings.beginGroup("childEvaluators");
    foreach (const QString &evaluatorGroup, settings.childGroups()) {
        ret.appendEvaluator(StateEvaluator::loadFromSettings(settings, evaluatorGroup));
    }
    settings.endGroup();
    settings.endGroup();
    return ret;
}


