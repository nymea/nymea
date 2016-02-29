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

/*!
    \class guhserver::StateEvaluator
    \brief This class helps to evaluate a \l{State} and .

    \ingroup rules
    \inmodule core

    The \l StateEvaluator class helps to evaluate a \l StateDescriptor and check if all \l {State}{States}
    from the given \l StateDescriptor are valid. A \l StateDescriptor is valid if conditions of the
    \l StateDescriptor are true.

    \sa StateDescriptor, State, RuleEngine
*/


#include "stateevaluator.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "loggingcategories.h"
#include "guhsettings.h"

namespace guhserver {

/*! Constructs a new StateEvaluator for the given \a stateDescriptor. */
StateEvaluator::StateEvaluator(const StateDescriptor &stateDescriptor):
    m_stateDescriptor(stateDescriptor),
    m_operatorType(Types::StateOperatorAnd)
{
}

/*! Constructs a new StateEvaluator for the given \a childEvaluators and \a stateOperator. */
StateEvaluator::StateEvaluator(QList<StateEvaluator> childEvaluators, Types::StateOperator stateOperator):
    m_stateDescriptor(),
    m_childEvaluators(childEvaluators),
    m_operatorType(stateOperator)
{
}

/*! Returns the \l StateDescriptor of this \l StateEvaluator. */
StateDescriptor StateEvaluator::stateDescriptor() const
{
    return m_stateDescriptor;
}

/*! Returns the list of child \l {StateEvaluator}{StateEvaluators} of this \l StateEvaluator. */
QList<StateEvaluator> StateEvaluator::childEvaluators() const
{
    return m_childEvaluators;
}

/*! Sets the list of child evaluators of this \l StateEvaluator to the given \a stateEvaluators.*/
void StateEvaluator::setChildEvaluators(const QList<StateEvaluator> &stateEvaluators)
{
    m_childEvaluators = stateEvaluators;
}

/*! Appends the given \a stateEvaluator to the child evaluators of this \l StateEvaluator.
    \sa childEvaluators()
*/
void StateEvaluator::appendEvaluator(const StateEvaluator &stateEvaluator)
{
    m_childEvaluators.append(stateEvaluator);
}

/*! Returns the \l {Types::StateOperator}{StateOperator} for this \l StateEvaluator.*/
Types::StateOperator StateEvaluator::operatorType() const
{
    return m_operatorType;
}

/*! Sets the \l {Types::StateOperator}{StateOperator} for this \l StateEvaluator to the given.
 * \a operatorType. This operator will be used to evaluate the child evaluator list.*/
void StateEvaluator::setOperatorType(Types::StateOperator operatorType)
{
    m_operatorType = operatorType;
}

/*! Returns true, if all child evaluator conditions are true depending on the \l {Types::StateOperator}{StateOperator}.*/
bool StateEvaluator::evaluate() const
{
    if (m_stateDescriptor.isValid()) {
        Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(m_stateDescriptor.deviceId());
        if (!device) {
            qCWarning(dcRuleEngine) << "Device not existing!";
            return false;
        }
        if (!device->hasState(m_stateDescriptor.stateTypeId())) {
            qCWarning(dcRuleEngine) << "Device found, but it does not appear to have such a state!";
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

/*! Returns true if this \l StateEvaluator has a \l Device in it with the given \a deviceId. */
bool StateEvaluator::containsDevice(const DeviceId &deviceId) const
{
    if (m_stateDescriptor.deviceId() == deviceId)
        return true;

    foreach (const StateEvaluator &childEvaluator, m_childEvaluators) {
        if (childEvaluator.containsDevice(deviceId)) {
            return true;
        }
    }
    return false;
}

/*! Removes a \l Device with the given \a deviceId from this \l StateEvaluator. */
void StateEvaluator::removeDevice(const DeviceId &deviceId)
{
    if (m_stateDescriptor.deviceId() == deviceId)
        m_stateDescriptor = StateDescriptor();

    for (int i = 0; i < m_childEvaluators.count(); i++) {
        m_childEvaluators[i].removeDevice(deviceId);
    }
}

/*! This method will be used to save this \l StateEvaluator to the given \a settings.
    The \a groupName will normally be the corresponding \l Rule. */
void StateEvaluator::dumpToSettings(GuhSettings &settings, const QString &groupName) const
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

/*! This method will be used to load a \l StateEvaluator from the given \a settings.
    The \a groupName will be the corresponding \l RuleId. Returns the loaded \l StateEvaluator. */
StateEvaluator StateEvaluator::loadFromSettings(GuhSettings &settings, const QString &groupName)
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

/*! Returns true, if all child evaluators are valid, the devices exist and all descriptors are in allowed paramerters.*/
bool StateEvaluator::isValid() const
{
    if (m_stateDescriptor.isValid()) {
        Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(m_stateDescriptor.deviceId());
        if (!device) {
            qCWarning(dcRuleEngine) << "State evaluator device does not exist!";
            return false;
        }

        if (!device->hasState(m_stateDescriptor.stateTypeId())) {
            qCWarning(dcRuleEngine) << "State evaluator device found, but it does not appear to have such a state!";
            return false;
        }

        DeviceClass deviceClass = GuhCore::instance()->deviceManager()->findDeviceClass(device->deviceClassId());
        foreach (const StateType &stateType, deviceClass.stateTypes()) {
            if (stateType.id() == m_stateDescriptor.stateTypeId()) {

                if (!m_stateDescriptor.stateValue().canConvert(stateType.type())) {
                    qCWarning(dcRuleEngine) << "Wrong state value for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Expected:" << QVariant::typeToName(stateType.type());
                    return false;
                }

                if (!m_stateDescriptor.stateValue().convert(stateType.type())) {
                    qCWarning(dcRuleEngine) << "Could not convert value of state descriptor" << m_stateDescriptor.stateTypeId() << " to:" << QVariant::typeToName(stateType.type()) << " Got:" << m_stateDescriptor.stateValue();
                    return false;
                }

                if (stateType.maxValue().isValid() && m_stateDescriptor.stateValue() > stateType.maxValue()) {
                    qCWarning(dcRuleEngine) << "Value out of range for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Max:" << stateType.maxValue();
                    return false;
                }

                if (stateType.minValue().isValid() && m_stateDescriptor.stateValue() < stateType.minValue()) {
                    qCWarning(dcRuleEngine) << "Value out of range for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Min:" << stateType.minValue();
                    return false;
                }

                if (!stateType.possibleValues().isEmpty() && !stateType.possibleValues().contains(m_stateDescriptor.stateValue())) {
                    QStringList possibleValues;
                    foreach (const QVariant &value, stateType.possibleValues()) {
                        possibleValues.append(value.toString());
                    }

                    qCWarning(dcRuleEngine) << "Value not in possible values for state type" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Possible values:" << possibleValues.join(", ");
                    return false;
                }
            }
        }

    }

    if (m_operatorType == Types::StateOperatorOr) {
        foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
            if (stateEvaluator.isValid()) {
                return true;
            }
        }
        return false;
    }

    foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
        if (!stateEvaluator.isValid()) {
            return false;
        }
    }
    return true;
}

}
