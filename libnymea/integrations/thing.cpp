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

/*!
  \class Thing
  \brief The class that represents devices and services in nymea.

  \ingroup things
  \inmodule libnymea

  This class holds the values for configured things. It is associated with a \{ThingClass} which
  can be used to get more details about the thing.

  \sa ThingClass, ThingDescriptor
*/

/*! \enum Thing::ThingError

    This enum type specifies the errors that can happen when working with \l{Thing}{Things}.

    \value ThingErrorNoError
        No Error. Everything went fine.
    \value ThingErrorPluginNotFound
        Couldn't find the Plugin for the given id.
    \value ThingErrorVendorNotFound
        Couldn't find the Vendor for the given id.
    \value ThingErrorThingNotFound
        Couldn't find a \l{Thing} for the given id.
    \value ThingErrorThingClassNotFound
        Couldn't find a \l{ThingClass} for the given id.
    \value ThingErrorActionTypeNotFound
        Couldn't find the \l{ActionType} for the given id.
    \value ThingErrorStateTypeNotFound
        Couldn't find the \l{StateType} for the given id.
    \value ThingErrorEventTypeNotFound
        Couldn't find the \l{EventType} for the given id.
    \value ThingErrorThingDescriptorNotFound
        Couldn't find the \l{ThingDescriptor} for the given id.
    \value ThingErrorMissingParameter
        Parameters do not comply to the template.
    \value ThingErrorInvalidParameter
        One of the given parameter is not valid.
    \value ThingErrorSetupFailed
        Error setting up the \l{Thing}. It will not be functional.
    \value ThingErrorDuplicateUuid
        Error setting up the \l{Thing}. The given ThingId already exists.
    \value ThingErrorCreationMethodNotSupported
        Error setting up the \l{Thing}. This \l{ThingClass}{CreateMethod} is not supported for this \l{Thing}.
    \value ThingErrorSetupMethodNotSupported
        Error setting up the \l{Thing}. This \l{ThingClass}{SetupMethod} is not supported for this \l{Thing}.
    \value ThingErrorHardwareNotAvailable
        The Hardware of the \l{Thing} is not available.
    \value ThingErrorHardwareFailure
        The Hardware of the \l{Thing} has an error.
    \value ThingErrorAsync
        The response of the \l{Thing} will be asynchronously.
    \value ThingErrorThingInUse
        The \l{Thing} is currently bussy.
    \value ThingErrorPairingTransactionIdNotFound
        Couldn't find the PairingTransactionId for the given id.
    \value ThingErrorAuthentificationFailure
        The authentication failed when communicating with the device or service.
    \value ThingErrorThingIsChild
        The thing is a child and can not be used in this way.
    \value ThingErrorThingInRule
        The thing is in a rule and can not be deleted withou \l{nymeaserver::RuleEngine::RemovePolicy}.
    \value ThingErrorParameterNotWritable
        One of the given thing params is not writable.
*/

/*! \enum Thing::ThingSetupStatus

    This enum type specifies the setup status of a \l{Thing}.

    \value ThingSetupStatusNone
        The thing has not been set up yet.
    \value ThingSetupStatusInProgress
        The setup for this thing is currently in progress.
    \value ThingSetupStatusComplete
        The setup for this thing has been completed.
    \value ThingSetupStatusFailed
        The setup for this thing failed.
*/

/*! \fn void Thing::stateValueChanged(const StateTypeId &stateTypeId, const QVariant &value)
    This signal is emitted when the \l{State} with the given \a stateTypeId changed.
    The \a value parameter describes the new value of the State.
*/

/*! \fn void settingChanged(const ParamTypeId &paramTypeId, const QVariant &value)
    This signal is emitted whenever a setting is changed.
*/

/*! \fn void nameChanged()
    This signal is emitted when the device name is changed.
    Note that the name can be changed by the plugin, as well as the user. Integration plugins
    should connect to this signal for their things if it possible to set the name on the actual device
    or service. If the the device or service does not support changing the name, this will only be used
    for nymea's internal representation and is not required to be handled by the plugin.
*/

/*! \fn void setupStatusChanged()
    This signal is emitted when the setup status of a thing changes.
*/

#include "thing.h"
#include "types/event.h"
#include "loggingcategories.h"
#include "statevaluefilters/statevaluefilteradaptive.h"
#include "thingutils.h"

#include <QJsonDocument>
#include <QDebug>

/*! Construct a Thing with the given \a pluginId, \a id, \a thingClassId and \a parent. */
Thing::Thing(const PluginId &pluginId, const ThingClass &thingClass, const ThingId &id, QObject *parent):
    QObject(parent),
    m_thingClass(thingClass),
    m_pluginId(pluginId),
    m_id(id)
{

}

/*! Construct a Thing with the given \a pluginId, \a thingClassId and \a parent. A new ThingId will be created for this Thing. */
Thing::Thing(const PluginId &pluginId, const ThingClass &thingClass, QObject *parent):
    QObject(parent),
    m_thingClass(thingClass),
    m_pluginId(pluginId),
    m_id(ThingId::createThingId())
{

}

Thing::~Thing()
{
    qDeleteAll(m_stateValueFilters);
    m_stateValueFilters.clear();
}

/*! Returns the id of this thing. */
ThingId Thing::id() const
{
    return m_id;
}

/*! Returns the thingClassId of the associated \l{ThingClass}. */
ThingClassId Thing::thingClassId() const
{
    return m_thingClass.id();
}

/*! Returns the id of the \l{ThingPlugin} this thing is managed by. */
PluginId Thing::pluginId() const
{
    return m_pluginId;
}

/*! Returns the \l{ThingClass} of this thing. */
ThingClass Thing::thingClass() const
{
    return m_thingClass;
}

/*! Returns the name of this Thing. This is visible to the user. */
QString Thing::name() const
{
    return m_name;
}

/*! Set the \a name for this thing. This is visible to the user.*/
void Thing::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

/*! Returns the parameter of this thing. It must match the parameter description in the associated \l{ThingClass}. */
ParamList Thing::params() const
{
    return m_params;
}

/*! Sets the \a params of this thing. It must match the parameter description in the associated \l{ThingClass}. */
void Thing::setParams(const ParamList &params)
{
    m_params = params;
}

/*! Returns the value of the \l{Param} of this thing with the given \a paramTypeId. */
QVariant Thing::paramValue(const ParamTypeId &paramTypeId) const
{
    foreach (const Param &param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            return param.value();
        }
    }
    return QVariant();
}

QVariant Thing::paramValue(const QString &paramName) const
{
    ParamTypeId paramTypeId = m_thingClass.paramTypes().findByName(paramName).id();
    return paramValue(paramTypeId);
}

/*! Sets the \a value of the \l{Param} with the given \a paramTypeId. */
void Thing::setParamValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    ParamList params;
    foreach (Param param, m_params) {
        if (param.paramTypeId() == paramTypeId) {
            param.setValue(value);
        }
        params << param;
    }
    m_params = params;
}

void Thing::setParamValue(const QString &paramName, const QVariant &value)
{
    ParamTypeId paramTypeId = m_thingClass.paramTypes().findByName(paramName).id();
    setParamValue(paramTypeId, value);
}

ParamList Thing::settings() const
{
    return m_settings;
}

bool Thing::hasSetting(const ParamTypeId &paramTypeId) const
{
    return m_settings.hasParam(paramTypeId);
}

void Thing::setSettings(const ParamList &settings)
{
    m_settings = settings;
    foreach (const Param &param, m_settings) {
        emit settingChanged(param.paramTypeId(), param.value());
    }
}

QVariant Thing::setting(const ParamTypeId &paramTypeId) const
{
    foreach (Param setting, m_settings) {
        if (setting.paramTypeId() == paramTypeId) {
            return setting.value();
        }
    }
    return QVariant();
}

QVariant Thing::setting(const QString &paramName) const
{
    ParamTypeId paramTypeId = m_thingClass.settingsTypes().findByName(paramName).id();
    return setting(paramTypeId);
}

void Thing::setSettingValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    ParamList settings;
    bool found = false;
    bool changed = false;
    foreach (Param param, m_settings) {
        if (param.paramTypeId() == paramTypeId) {
            found = true;
            if (param.value() != value) {
                param.setValue(value);
                changed = true;
            }
        }
        settings << param;
    }
    if (!found) {
        qCWarning(dcThingManager()) << this << "does not have a setting with id" << paramTypeId;
        return;
    }
    if (changed) {
        m_settings = settings;
        emit settingChanged(paramTypeId, value);
    }
}

void Thing::setSettingValue(const QString &paramName, const QVariant &value)
{
    ParamTypeId paramTypeId = m_thingClass.settingsTypes().findByName(paramName).id();
    setSettingValue(paramTypeId, value);
}

/*! Returns the states of this thing. It must match the \l{StateType} description in the associated \l{ThingClass}. */
States Thing::states() const
{
    return m_states;
}

/*! Returns true, a \l{Param} with the given \a paramTypeId exists for this thing. */
bool Thing::hasParam(const ParamTypeId &paramTypeId) const
{
    return m_params.hasParam(paramTypeId);
}

/*! Set the \l{State}{States} of this \l{Thing} to the given \a states.*/
void Thing::setStates(const States &states)
{
    m_states = states;
}

/*! Returns true, a \l{State} with the state given by \a stateTypeId exists for this thing. */
bool Thing::hasState(const StateTypeId &stateTypeId) const
{
    foreach (const State &state, m_states) {
        if (state.stateTypeId() == stateTypeId) {
            return true;
        }
    }
    return false;
}

/*! Finds the \l{State} matching the given \a stateTypeId in this thing and returns the current value. */
bool Thing::hasState(const QString &stateName) const
{
    StateTypeId stateTypeId = m_thingClass.stateTypes().findByName(stateName).id();
    return hasState(stateTypeId);
}

/*! Finds the \l{State} matching the given \a stateTypeId in this thing and returns the current value. */
QVariant Thing::stateValue(const StateTypeId &stateTypeId) const
{
    foreach (const State &state, m_states) {
        if (state.stateTypeId() == stateTypeId) {
            return state.value();
        }
    }
    return QVariant();
}

/*! Finds the \l{State} matching the given \a stateName in this thing and returns the current value. */
QVariant Thing::stateValue(const QString &stateName) const
{
    return stateValue(m_thingClass.stateTypes().findByName(stateName).id());
}

/*! Sets the value for the \l{State} matching the given \a stateTypeId in this thing to value. */
void Thing::setStateValue(const StateTypeId &stateTypeId, const QVariant &value)
{
    StateType stateType = m_thingClass.stateTypes().findById(stateTypeId);
    if (!stateType.isValid()) {
        qCWarning(dcThing()) << "No such state type" << stateTypeId.toString() << "in" << this;
        return;
    }
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            QVariant newValue = value;
            if (!newValue.convert(stateType.type())) {
                qCWarning(dcThing()).nospace() << this << ": Invalid value " << value << " for state " << stateType.name() << ". Type mismatch. Expected type: " << QVariant::typeToName(stateType.type()) << " (Discarding change)";
                return;
            }
            State state = m_states.at(i);
            if (state.minValue().isValid() && ThingUtils::variantLessThan(value, state.minValue())) {
                qCWarning(dcThing()).nospace() << this << ": Invalid value " << value << " for state " << stateType.name() << ". Out of range: " << state.minValue() << " - " << state.maxValue() << " (Correcting to closest value within range)";
                newValue = state.minValue();
            }
            if (state.maxValue().isValid() && ThingUtils::variantGreaterThan(value, state.maxValue())) {
                qCWarning(dcThing()).nospace() << this << ": Invalid value " << value << " for state " << stateType.name() << ". Out of range: " << state.minValue() << " - " << state.maxValue() << " (Correcting to closest value within range)";
                newValue = state.maxValue();
            }
            if (!stateType.possibleValues().isEmpty() && !stateType.possibleValues().contains(value)) {
                qCWarning(dcThing()).nospace() << this << ": Invalid value " << value << " for state " << stateType.name() << ". Not an accepted value. Possible values: " << stateType.possibleValues() << " (Discarding change)";
                return;
            }

            StateValueFilter *filter = m_stateValueFilters.value(stateTypeId);
            if (filter) {
                filter->addValue(newValue);
                newValue = filter->filteredValue();
                newValue.convert(stateType.type());
            }

            QVariant oldValue = m_states.at(i).value();
            if (oldValue == newValue) {
                qCDebug(dcThing()).nospace() << this << ": Discarding state change for " << stateType.name() << " as the value did not actually change. Old value:" << oldValue << "New value:" << newValue;
                return;
            }

            qCDebug(dcThing()).nospace() << this << ": State " << stateType.name() << " changed from " << oldValue << " to " << newValue;
            m_states[i].setValue(newValue);
            emit stateValueChanged(stateTypeId, newValue, m_states.at(i).minValue(), m_states.at(i).maxValue(), m_states.at(i).possibleValues());
            return;
        }
    }
    Q_ASSERT_X(false, m_name.toUtf8(), QString("Failed setting state %1 to %2").arg(stateType.name()).arg(value.toString()).toUtf8());
    qCWarning(dcThing()).nospace() << this << ": Failed setting state " << stateType.name() << " to " << value;
}

/*! Sets the value for the \l{State} matching the given \a stateName in this thing to value. */
void Thing::setStateValue(const QString &stateName, const QVariant &value)
{
    StateTypeId stateTypeId = m_thingClass.stateTypes().findByName(stateName).id();
    if (stateTypeId.isNull()) {
        qCWarning(dcThing()) << "No such state" << stateName << "in" << m_name << "(" + thingClass().name() + ")";
        return;
    }

    setStateValue(stateTypeId, value);
}

/*! Sets the minimum value for the \l{State} matching the given \a stateTypeId in this thing to value. */
void Thing::setStateMinValue(const StateTypeId &stateTypeId, const QVariant &minValue)
{
    StateType stateType = m_thingClass.stateTypes().findById(stateTypeId);
    if (!stateType.isValid()) {
        qCWarning(dcThing()) << "No such state type" << stateTypeId.toString() << "in" << m_name << "(" + thingClass().name() + ")";
        return;
    }
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            QVariant newMin = minValue.isValid() ? minValue : stateType.minValue();

            if (newMin == m_states.at(i).minValue()) {
                return;
            }

            m_states[i].setMinValue(newMin);

            // Sanity check for max >= min
            if (ThingUtils::variantLessThan(m_states.at(i).maxValue(), newMin)) {
                qCWarning(dcThing()).nospace() << this << ": Adjusting state maximum value for " << stateType.name() << " from " << m_states.at(i).maxValue() << " to new minimum value of " << newMin;
                m_states[i].setMaxValue(newMin);
            }
            if (ThingUtils::variantLessThan(m_states.at(i).value(), newMin)) {
                qCInfo(dcThing()).nospace() << this << ": Adjusting state value for " << stateType.name() << " from " << m_states.at(i).value() << " to new minimum value of " << newMin;
                m_states[i].setValue(newMin);
            }

            emit stateValueChanged(stateTypeId, m_states.at(i).value(), m_states.at(i).minValue(), m_states.at(i).maxValue(), m_states.at(i).possibleValues());
            return;
        }
    }
    Q_ASSERT_X(false, m_name.toUtf8(), QString("Failed setting minimum state value %1 to %2").arg(stateType.name()).arg(minValue.toString()).toUtf8());
    qCWarning(dcThing()).nospace() << this << ": Failed setting minimum state value " << stateType.name() << " to " << minValue;
}

/*! Sets the minimum value for the \l{State} matching the given \a stateName in this thing to value. */
void Thing::setStateMinValue(const QString &stateName, const QVariant &minValue)
{
    StateTypeId stateTypeId = m_thingClass.stateTypes().findByName(stateName).id();
    setStateMinValue(stateTypeId, minValue);
}

/*! Sets the maximum value for the \l{State} matching the given \a stateTypeId in this thing to value. */
void Thing::setStateMaxValue(const StateTypeId &stateTypeId, const QVariant &maxValue)
{
    StateType stateType = m_thingClass.stateTypes().findById(stateTypeId);
    if (!stateType.isValid()) {
        qCWarning(dcThing()) << "No such state type" << stateTypeId.toString() << "in" << m_name << "(" + thingClass().name() + ")";
        return;
    }
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            QVariant newMax = maxValue.isValid() ? maxValue : stateType.maxValue();

            if (newMax == m_states.at(i).maxValue()) {
                return;
            }

            m_states[i].setMaxValue(newMax);

            if (newMax.isValid()) {
                // Sanity check for min <= max
                if (ThingUtils::variantGreaterThan(m_states.at(i).minValue(), newMax)) {
                    qCWarning(dcThing()).nospace() << this << ": Adjusting minimum state value for " << stateType.name() << " from " << m_states.at(i).minValue() << " to new maximum value of " << newMax;
                    m_states[i].setMinValue(newMax);
                }

                if (ThingUtils::variantGreaterThan(m_states.at(i).value(), newMax)) {
                    qCInfo(dcThing()).nospace() << this << ": Adjusting state value for " << stateType.name() << " from " << m_states.at(i).value() << " to new maximum value of " << newMax;
                    m_states[i].setValue(maxValue);
                }
            }

            emit stateValueChanged(stateTypeId, m_states.at(i).value(), m_states.at(i).minValue(), m_states.at(i).maxValue(), m_states.at(i).possibleValues());
            return;
        }
    }
    Q_ASSERT_X(false, m_name.toUtf8(), QString("Failed setting maximum state value %1 to %2").arg(stateType.name()).arg(maxValue.toString()).toUtf8());
    qCWarning(dcThing()).nospace() << this << ": Failed setting maximum state value " << stateType.name() << " to " << maxValue;
}

/*! Sets the maximum value for the \l{State} matching the given \a stateName in this thing to value. */
void Thing::setStateMaxValue(const QString &stateName, const QVariant &maxValue)
{
    StateTypeId stateTypeId = m_thingClass.stateTypes().findByName(stateName).id();
    setStateMaxValue(stateTypeId, maxValue);
}

void Thing::setStateMinMaxValues(const StateTypeId &stateTypeId, const QVariant &minValue, const QVariant &maxValue)
{
    StateType stateType = m_thingClass.stateTypes().findById(stateTypeId);
    if (!stateType.isValid()) {
        qCWarning(dcThing()) << "No such state type" << stateTypeId.toString() << "in" << m_name << "(" + thingClass().name() + ")";
        return;
    }
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            QVariant newMin = minValue.isValid() ? minValue : stateType.minValue();
            QVariant newMax = maxValue.isValid() ? maxValue : stateType.maxValue();

            if (newMin == m_states.at(i).minValue() && newMax == m_states.at(i).maxValue()) {
                return;
            }

            m_states[i].setMinValue(newMin);
            m_states[i].setMaxValue(newMax);

            if (newMax.isValid() || newMax.isValid()) {
                // Sanity check for min <= max
                if (ThingUtils::variantGreaterThan(newMin, newMax)) {
                    qCWarning(dcThing()).nospace() << this << ": Adjusting maximum state value for " << stateType.name() << " from " << m_states.at(i).maxValue() << " to new minimum value of " << newMax;
                    m_states[i].setMaxValue(newMin);
                }

                if (ThingUtils::variantLessThan(m_states.at(i).value(), m_states.at(i).minValue())) {
                    qCInfo(dcThing()).nospace() << this << ": Adjusting state value for " << stateType.name() << " from " << m_states.at(i).value() << " to new minimum value of " << m_states.at(i).minValue();
                    m_states[i].setValue(m_states.at(i).minValue());
                }
                if (ThingUtils::variantGreaterThan(m_states.at(i).value(), m_states.at(i).maxValue())) {
                    qCInfo(dcThing()).nospace() << this << ": Adjusting state value for " << stateType.name() << " from " << m_states.at(i).value() << " to new maximum value of " << m_states.at(i).maxValue();
                    m_states[i].setValue(m_states.at(i).maxValue());
                }
            }

            emit stateValueChanged(stateTypeId, m_states.at(i).value(), m_states.at(i).minValue(), m_states.at(i).maxValue(), m_states.at(i).possibleValues());
            return;
        }
    }
    Q_ASSERT_X(false, m_name.toUtf8(), QString("Failed setting maximum state value %1 to %2").arg(stateType.name()).arg(maxValue.toString()).toUtf8());
    qCWarning(dcThing()).nospace() << this << ": Failed setting maximum state value " << stateType.name() << " to " << maxValue;

}

void Thing::setStateMinMaxValues(const QString &stateName, const QVariant &minValue, const QVariant &maxValue)
{
    StateTypeId stateTypeId = m_thingClass.stateTypes().findByName(stateName).id();
    setStateMinMaxValues(stateTypeId, minValue, maxValue);
}

void Thing::setStatePossibleValues(const StateTypeId &stateTypeId, const QVariantList &values)
{
    StateType stateType = m_thingClass.stateTypes().findById(stateTypeId);
    if (!stateType.isValid()) {
        qCWarning(dcThing()) << "No such state type" << stateTypeId.toString() << "in" << m_name << "(" + thingClass().name() + ")";
        return;
    }
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            if (values == m_states.at(i).possibleValues()) {
                return;
            }

            m_states[i].setPossibleValues(values);

            if (!values.contains(m_states.value(i).value())) {
                if (values.contains(stateType.defaultValue())) {
                    qCInfo(dcThing()).nospace() << this << ": Adjusting state value for " << stateType.name() << " from " << m_states.at(i).value() << " to default value of " << stateType.defaultValue();
                    m_states[i].setValue(stateType.defaultValue());
                } else if (!values.isEmpty()) {
                    qCInfo(dcThing()).nospace() << this << ": Adjusting state value for " << stateType.name() << " from " << m_states.at(i).value() << " to new value of " << values.first();
                    m_states[i].setValue(values.first());
                }
            }
            emit stateValueChanged(stateTypeId, m_states.at(i).value(), m_states.at(i).minValue(), m_states.at(i).maxValue(), m_states.at(i).possibleValues());
            return;
        }
    }
    qCWarning(dcThing()).nospace() << this << ": Failed setting maximum state value " << stateType.name() << " to " << values;
    Q_ASSERT_X(false, m_name.toUtf8(), QString("Failed setting possible state values for %1 to %2").arg(stateType.name()).arg(QString(QJsonDocument::fromVariant(values).toJson())).toUtf8());

}

/*! Returns the \l{State} with the given \a stateTypeId of this thing. */
State Thing::state(const StateTypeId &stateTypeId) const
{
    for (int i = 0; i < m_states.count(); ++i) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            return m_states.at(i);
        }
    }
    return State(StateTypeId(), ThingId());
}

/*! Returns the \l{State} with the given name of this thing. */
State Thing::state(const QString &stateName) const
{
    StateTypeId stateTypeId = m_thingClass.stateTypes().findByName(stateName).id();
    return state(stateTypeId);
}

QList<StateTypeId> Thing::loggedStateTypeIds() const
{
    return m_loggedStateTypeIds;
}

QList<EventTypeId> Thing::loggedEventTypeIds() const
{
    return m_loggedEventTypeIds;
}

QList<ActionTypeId> Thing::loggedActionTypeIds() const
{
    return m_loggedActionTypeIds;
}

/*! Returns the \l{ThingId} of the parent of this thing. If the parentId
    is not set, this thing does not have a parent.
*/
ThingId Thing::parentId() const
{
    return m_parentId;
}

/*! Sets the \a parentId of this thing. If the parentId
    is not set, this thing does not have a parent.
*/
void Thing::setParentId(const ThingId &parentId)
{
    m_parentId = parentId;
}

/*! Returns true, if setup of this Thing is already completed. This method is deprecated, use setupStatus() instead. */
bool Thing::setupComplete() const
{
    return m_setupStatus == ThingSetupStatusComplete;
}

/*! Returns the setup error display message, if any. */
QString Thing::setupDisplayMessage() const
{
    return m_setupDisplayMessage;
}

/*! Emits an event from this thing to the system. */
void Thing::emitEvent(const EventTypeId &eventTypeId, const ParamList &params)
{
    emit eventTriggered(Event(eventTypeId, m_id, params));
}

/*! Emits an event from this thing to the system. */
void Thing::emitEvent(const QString &eventName, const ParamList &params)
{
    EventTypeId eventTypeId = m_thingClass.eventTypes().findByName(eventName).id();
    emit eventTriggered(Event(eventTypeId, m_id, params));
}

/*! Returns true if this thing has been auto-created (not created by the user) */
bool Thing::autoCreated() const
{
    return m_autoCreated;
}

/* Returns the current setup status. */
Thing::ThingSetupStatus Thing::setupStatus() const
{
    return m_setupStatus;
}

Thing::ThingError Thing::setupError() const
{
    return m_setupError;
}

void Thing::setSetupStatus(Thing::ThingSetupStatus status, Thing::ThingError setupError, const QString &displayMessage)
{
    m_setupStatus = status;
    m_setupError = setupError;
    m_setupDisplayMessage = displayMessage;
    emit setupStatusChanged();
}

void Thing::setLoggedStateTypeIds(const QList<StateTypeId> loggedStateTypeIds)
{
    m_loggedStateTypeIds = loggedStateTypeIds;
}

void Thing::setLoggedEventTypeIds(const QList<EventTypeId> loggedEventTypeIds)
{
    m_loggedEventTypeIds = loggedEventTypeIds;
}

void Thing::setLoggedActionTypeIds(const QList<ActionTypeId> loggedActionTypeIds)
{
    m_loggedActionTypeIds = loggedActionTypeIds;
}

void Thing::setStateValueFilter(const StateTypeId &stateTypeId, Types::StateValueFilter filter)
{
    for (int i = 0; i < m_states.count(); i++) {
        if (m_states.at(i).stateTypeId() == stateTypeId) {
            m_states[i].setFilter(filter);
            StateValueFilter *stateValueFilter = m_stateValueFilters.take(stateTypeId);
            if (stateValueFilter) {
                delete stateValueFilter;
            }
            if (filter == Types::StateValueFilterAdaptive) {
                m_stateValueFilters.insert(stateTypeId, new StateValueFilterAdaptive());
            }
        }
    }
}

Things::Things(const QList<Thing*> &other)
{
    foreach (Thing* thing, other) {
        this->append(thing);
    }
}

Thing *Things::findById(const ThingId &id)
{
    foreach (Thing *thing, *this) {
        if (thing->id() == id) {
            return thing;
        }
    }
    return nullptr;
}

/*! Find a certain thing by its \a params. All parameters must
    match or the thing will not be found.
*/
Thing *Things::findByParams(const ParamList &params) const
{
    foreach (Thing *thing, *this) {
        bool matching = true;
        foreach (const Param &param, params) {
            if (thing->paramValue(param.paramTypeId()) != param.value()) {
                matching = false;
            }
        }
        if (matching) {
            return thing;
        }
    }
    return nullptr;
}

QDebug operator<<(QDebug debug, Thing *thing)
{
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "Thing(" << thing->name();
    debug.nospace().noquote() << ", id: " << thing->id().toString();
    debug.nospace().noquote() << ", ThingClassId: " << thing->thingClassId().toString() << ")";
    return debug;
}

/*! Filter things by a parameter. Only things having a parameter of the given
    \a paramTypeId will be returned. If \a value is given and it is not null, only things
    with the given \a paramTypeId and the same \a value will be returned.
 */
Things Things::filterByParam(const ParamTypeId &paramTypeId, const QVariant &value)
{
    Things ret;
    foreach (Thing* thing, *this) {
        if (!thing->thingClass().paramTypes().findById(paramTypeId).isValid()) {
            continue;
        }
        if (!value.isNull() && thing->paramValue(paramTypeId) != value) {
            continue;
        }
        ret << thing;
    }
    return ret;
}

Things Things::filterByThingClassId(const ThingClassId &thingClassId)
{
    Things ret;
    foreach (Thing* thing, *this) {
        if (thing->thingClassId() == thingClassId) {
            ret << thing;
        }
    }
    return ret;
}

Things Things::filterByParentId(const ThingId &thingId)
{
    Things ret;
    foreach (Thing *thing, *this) {
        if (thing->parentId() == thingId) {
            ret << thing;
        }
    }
    return ret;
}

Things Things::filterByInterface(const QString &interface)
{
    Things ret;
    foreach (Thing *thing, *this) {
        if (thing->thingClass().interfaces().indexOf(interface) >= 0) {
            ret.append(thing);
        }
    }
    return ret;
}

QVariant Things::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Things::put(const QVariant &variant)
{
    append(variant.value<Thing*>());
}
