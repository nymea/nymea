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


#include "stateevaluator.h"
#include "nymeacore.h"
#include "integrations/thingmanager.h"
#include "loggingcategories.h"
#include "nymeasettings.h"

namespace nymeaserver {

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

void StateEvaluator::setStateDescriptor(const StateDescriptor &stateDescriptor)
{
    m_stateDescriptor = stateDescriptor;
}

StateEvaluators StateEvaluator::childEvaluators() const
{
    return m_childEvaluators;
}

void StateEvaluator::setChildEvaluators(const StateEvaluators &stateEvaluators)
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
    qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Evaluating: Operator type" << m_operatorType << "Valid descriptor:" << m_stateDescriptor.isValid() << "Childs:" << m_childEvaluators.count();
    bool descriptorMatching = true;
    if (m_stateDescriptor.isValid()) {
        descriptorMatching = evaluateDescriptor(m_stateDescriptor);
    }

    if (m_operatorType == Types::StateOperatorOr) {
        if (m_stateDescriptor.isValid() && descriptorMatching) {
            qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Descriptor is matching. Operator is OR => Evaluation result: true";
            return true;
        }
        foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
            if (stateEvaluator.evaluate()) {
                qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Child evaluator evaluated to true. Operator is OR => Evaluation result: true";
                return true;
            }
        }
        qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "No child evaluator evaluated to true => Evaluation result: false";
        return false;
    }

    if (!descriptorMatching) {
        qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "StateDescriptor not matching and operator is AND => Evaluation result: false";
        return false;
    }

    foreach (const StateEvaluator &stateEvaluator, m_childEvaluators) {
        if (!stateEvaluator.evaluate()) {
            qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "Child evaluator not matching => Evaluation result: false";
            return false;
        }
    }
    qCDebug(dcRuleEngineDebug()) << "StateEvaluator:" << this << "StateDescriptor and all child evaluators matching => Evaluation result: true";
    return true;
}

bool StateEvaluator::containsThing(const ThingId &thingId) const
{
    if (m_stateDescriptor.thingId() == thingId || m_stateDescriptor.valueThingId() == thingId)
        return true;

    foreach (const StateEvaluator &childEvaluator, m_childEvaluators) {
        if (childEvaluator.containsThing(thingId)) {
            return true;
        }
    }
    return false;
}

void StateEvaluator::removeThing(const ThingId &thingId)
{
    if (m_stateDescriptor.thingId() == thingId || m_stateDescriptor.valueThingId() == thingId)
        m_stateDescriptor = StateDescriptor();

    for (int i = 0; i < m_childEvaluators.count(); i++) {
        m_childEvaluators[i].removeThing(thingId);
    }
}

QList<ThingId> StateEvaluator::containedThings() const
{
    QList<ThingId> ret;
    if (!m_stateDescriptor.thingId().isNull()) {
        ret.append(m_stateDescriptor.thingId());
    }
    if (!m_stateDescriptor.valueThingId().isNull()) {
        ret.append(m_stateDescriptor.valueThingId());
    }
    foreach (const StateEvaluator &childEvaluator, m_childEvaluators) {
        ret.append(childEvaluator.containedThings());
    }
    return ret;
}

void StateEvaluator::dumpToSettings(NymeaSettings &settings, const QString &groupName) const
{
    settings.beginGroup(groupName);

    settings.beginGroup("stateDescriptor");
    settings.setValue("stateTypeId", m_stateDescriptor.stateTypeId().toString());
    settings.setValue("thingId", m_stateDescriptor.thingId().toString());
    settings.setValue("interface", m_stateDescriptor.interface());
    settings.setValue("interfaceState", m_stateDescriptor.interfaceState());
    settings.setValue("value", m_stateDescriptor.stateValue());
    settings.setValue("valueType", (int)m_stateDescriptor.stateValue().type());
    settings.setValue("valueThingId", m_stateDescriptor.valueThingId().toString());
    settings.setValue("valueStateTypeId", m_stateDescriptor.valueStateTypeId().toString());
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

StateEvaluator StateEvaluator::loadFromSettings(NymeaSettings &settings, const QString &groupName)
{
    settings.beginGroup(groupName);
    settings.beginGroup("stateDescriptor");
    StateTypeId stateTypeId(settings.value("stateTypeId").toString());
    ThingId thingId(settings.value("thingId").toString());
    if (thingId.isNull()) { // Retry with deviceId for backwards compatibility (<0.19)
        thingId = ThingId(settings.value("deviceId").toString());
    }
    QVariant stateValue = settings.value("value");
    if (settings.contains("valueType")) {
        QVariant::Type valueType = (QVariant::Type)settings.value("valueType").toInt();
        // Note: only warn, and continue with the QVariant guessed type
        if (valueType == QVariant::Invalid) {
            qCWarning(dcRuleEngine()) << "Could not load the value type of the state evaluator. The value type will be guessed by QVariant" << stateValue;
        } else if (!stateValue.canConvert(valueType)) {
            qCWarning(dcRuleEngine()) << "Could not convert the state evaluator value" << stateValue << "to the stored type" << valueType << ". The value type will be guessed by QVariant.";
        } else {
            stateValue.convert(valueType);
        }
    }

    ThingId valueThingId = settings.value("valueThingId").toUuid();
    StateTypeId valueStateTypeId = settings.value("valueStateTypeId").toUuid();

    QString interface = settings.value("interface").toString();
    QString interfaceState = settings.value("interfaceState").toString();
    Types::ValueOperator valueOperator = (Types::ValueOperator)settings.value("operator").toInt();
    StateDescriptor stateDescriptor;
    if (!thingId.isNull() && !stateTypeId.isNull()) {
        stateDescriptor = StateDescriptor(stateTypeId, thingId, stateValue, valueOperator);
    } else {
        stateDescriptor = StateDescriptor(interface, interfaceState, stateValue, valueOperator);
    }
    stateDescriptor.setValueThingId(valueThingId);
    stateDescriptor.setValueStateTypeId(valueStateTypeId);

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

bool StateEvaluator::isValid() const
{
    if (m_stateDescriptor.isValid()) {
        if (m_stateDescriptor.type() == StateDescriptor::TypeThing) {
            Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_stateDescriptor.thingId());
            if (!thing) {
                qCWarning(dcRuleEngine) << "State evaluator thing does not exist!";
                return false;
            }

            if (!thing->hasState(m_stateDescriptor.stateTypeId())) {
                qCWarning(dcRuleEngine) << "State evaluator thing found, but it does not appear to have such a state!";
                return false;
            }

            ThingClass thingClass = NymeaCore::instance()->thingManager()->findThingClass(thing->thingClassId());
            foreach (const StateType &stateType, thingClass.stateTypes()) {
                if (stateType.id() == m_stateDescriptor.stateTypeId()) {

                    // If comparing to a static value
                    if (!m_stateDescriptor.stateValue().isNull()) {

                        QVariant stateValue = m_stateDescriptor.stateValue();
                        if (!stateValue.convert(stateType.type())) {
                            qCWarning(dcRuleEngine) << "Could not convert value of state descriptor" << m_stateDescriptor.stateTypeId() << " to:" << QVariant::typeToName(stateType.type()) << " Got:" << m_stateDescriptor.stateValue();
                            return false;
                        }
                        if (stateType.maxValue().isValid() && stateValue > stateType.maxValue()) {
                            qCWarning(dcRuleEngine) << "Value out of range for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Max:" << stateType.maxValue();
                            return false;
                        }

                        if (stateType.minValue().isValid() && stateValue < stateType.minValue()) {
                            qCWarning(dcRuleEngine) << "Value out of range for state descriptor" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Min:" << stateType.minValue();
                            return false;
                        }

                        if (!stateType.possibleValues().isEmpty() && !stateType.possibleValues().contains(stateValue)) {
                            QStringList possibleValues;
                            foreach (const QVariant &value, stateType.possibleValues()) {
                                possibleValues.append(value.toString());
                            }

                            qCWarning(dcRuleEngine) << "Value not in possible values for state type" << m_stateDescriptor.stateTypeId() << " Got:" << m_stateDescriptor.stateValue() << " Possible values:" << possibleValues.join(", ");
                            return false;
                        }

                    // if comparing to another state
                    } else if (!m_stateDescriptor.valueThingId().isNull() && !m_stateDescriptor.valueStateTypeId().isNull()) {
                        Thing *valueThing = NymeaCore::instance()->thingManager()->findConfiguredThing(m_stateDescriptor.valueThingId());
                        if (!valueThing) {
                            qCWarning(dcRuleEngine()) << "State descriptor valueThing does not exist" << m_stateDescriptor.valueThingId().toString();
                            return false;
                        }
                        StateType valueStateType = valueThing->thingClass().stateTypes().findById(m_stateDescriptor.valueStateTypeId());
                        if (!valueStateType.isValid()) {
                            qCWarning(dcRuleEngine()) << "State descriptor value state type" << m_stateDescriptor.valueStateTypeId().toString() << "does not exist in thing" << valueThing->name();
                            return false;
                        }
                    } else {
                        qCWarning(dcRuleEngine()) << "State descriptor contains neither stateValue nor valueThingId and valueStateTypeId";
                        return false;
                    }
                }
            }
        } else { // TypeInterface
            Interface iface = NymeaCore::instance()->thingManager()->supportedInterfaces().findByName(m_stateDescriptor.interface());
            if (!iface.isValid()) {
                qWarning(dcRuleEngine()) << "No such interface:" << m_stateDescriptor.interface();
                return false;
            }
            if (iface.stateTypes().findByName(m_stateDescriptor.interfaceState()).name().isEmpty()) {
                qWarning(dcRuleEngine()) << "Interface" << iface.name() << "has no such state:" << m_stateDescriptor.interfaceState();
                return false;
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

bool StateEvaluator::isEmpty() const
{
    return !m_stateDescriptor.isValid() && m_childEvaluators.isEmpty();
}

bool StateEvaluator::evaluateDescriptor(const StateDescriptor &descriptor) const
{
    if (descriptor.type() == StateDescriptor::TypeThing) {
        qCDebug(dcRuleEngineDebug()) << "Evaluating thing based state descriptor";

        Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(descriptor.thingId());
        if (!thing) {
            qCWarning(dcRuleEngine()) << "Thing listed in state descriptor not found in system.";
            return false;
        }
        State state = thing->state(descriptor.stateTypeId());
        if (state.stateTypeId().isNull()) {
            qCWarning(dcRuleEngine()) << "State" << descriptor.stateTypeId() << "not found in thing" << thing->name() << thing->id().toString();
            return false;
        }

        if (!descriptor.stateValue().isNull()) {
            QVariant convertedValue = descriptor.stateValue();
            bool res = convertedValue.convert(state.value().type());
            if (!res) {
                return false;
            }
            switch (descriptor.operatorType()) {
            case Types::ValueOperatorEquals:
                return state.value() == convertedValue;
            case Types::ValueOperatorGreater:
                return state.value() > convertedValue;
            case Types::ValueOperatorGreaterOrEqual:
                return state.value() >= convertedValue;
            case Types::ValueOperatorLess:
                return state.value() < convertedValue;
            case Types::ValueOperatorLessOrEqual:
                return state.value() <= convertedValue;
            case Types::ValueOperatorNotEquals:
                return state.value() != convertedValue;
            }

        } else if (!descriptor.valueThingId().isNull() && !descriptor.valueStateTypeId().isNull()) {
            Thing *valueThing = NymeaCore::instance()->thingManager()->findConfiguredThing(descriptor.valueThingId());
            if (!valueThing) {
                qCWarning(dcRuleEngine()) << "Thing" << descriptor.valueThingId().toString() << "defined in statedescriptor value but not found in system.";
                return false;
            }
            State valueState = valueThing->state(descriptor.valueStateTypeId());
            if (valueState.stateTypeId().isNull()) {
                qCWarning(dcRuleEngine()) << "State" << descriptor.valueStateTypeId().toString() << "defined in statedescriptor value not found in thing" << thing->name() << thing->id().toString();
                return false;
            }

            qCDebug(dcRuleEngineDebug()) << "Comparing" << state.value() << "to" << valueState.value() << "with operator" << descriptor.operatorType();
            switch (descriptor.operatorType()) {
            case Types::ValueOperatorEquals:
                return state.value() == valueState.value();
            case Types::ValueOperatorGreater:
                return state.value() > valueState.value();
            case Types::ValueOperatorGreaterOrEqual:
                return state.value() >= valueState.value();
            case Types::ValueOperatorLess:
                return state.value() < valueState.value();
            case Types::ValueOperatorLessOrEqual:
                return state.value() <= valueState.value();
            case Types::ValueOperatorNotEquals:
                return state.value() != valueState.value();
            }
        }

    } else { // Interface based
        qCDebug(dcRuleEngineDebug()) << "Evaluating interface based state descriptor" << descriptor.interface();

        foreach (Thing* thing, NymeaCore::instance()->thingManager()->configuredThings()) {
            if (thing->thingClass().interfaces().contains(descriptor.interface())) {
                qCDebug(dcRuleEngineDebug()) << "Thing" << thing->name() << "has matching interface";
                StateType stateType = thing->thingClass().stateTypes().findByName(descriptor.interfaceState());
                State state = thing->state(stateType.id());
                // Generate a thing based state descriptor and run again
                StateDescriptor temporaryDescriptor(stateType.id(), thing->id(), descriptor.stateValue(), descriptor.operatorType());
                temporaryDescriptor.setValueThingId(descriptor.valueThingId());
                temporaryDescriptor.setValueStateTypeId(descriptor.valueStateTypeId());
                if (evaluateDescriptor(temporaryDescriptor)) {
                    return true;
                }
            }
        }
    }

    return false;
}

QDebug operator<<(QDebug dbg, const StateEvaluator &stateEvaluator)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "StateEvaluator: Operator:" << stateEvaluator.operatorType() << Qt::endl << "  " << stateEvaluator.stateDescriptor() << Qt::endl;
    for (int i = 0; i < stateEvaluator.childEvaluators().count(); i++) {
        dbg.nospace() << "    " << i << ": " << stateEvaluator.childEvaluators().at(i);
    }
    return dbg;
}

StateEvaluators::StateEvaluators()
{

}

StateEvaluators::StateEvaluators(const QList<StateEvaluator> &other): QList<StateEvaluator>(other)
{

}

QVariant StateEvaluators::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void StateEvaluators::put(const QVariant &variant)
{
    append(variant.value<StateEvaluator>());
}

}
