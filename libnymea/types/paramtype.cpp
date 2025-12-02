// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class ParamType
    \brief Describes a certain ParamType.

    \ingroup nymea-types
    \inmodule libnymea

    \sa Device, Param, ParamDescriptor
*/

/*! \fn ParamType::ParamType();
    Constructs a new ParamType which is initially not valid.

    \sa isValid()
*/


#include "paramtype.h"

/*! Constructs a ParamType object with the given \a id, \a name, \a type and \a defaultValue. */
ParamType::ParamType(const ParamTypeId &id, const QString &name, const QMetaType::Type type, const QVariant &defaultValue):
    m_id(id),
    m_name(name),
    m_index(0),
    m_type(type),
    m_defaultValue(defaultValue),
    m_inputType(Types::InputTypeNone),
    m_unit(Types::UnitNone),
    m_readOnly(false)
{

}

/*! Returns the \l{ParamTypeId} of this ParamType. */
ParamTypeId ParamType::id() const
{
    return m_id;
}

/*! Returns the name of this ParamType. */
QString ParamType::name() const
{
    return m_name;
}

/*! Sets the name of this ParamType to the given \a name. */
void ParamType::setName(const QString &name)
{
    m_name = name;
}

/*! Returns the displayName of this ParamType, to be shown to the user, translated. */
QString ParamType::displayName() const
{
    return m_displayName;
}

/*! Sets the \a displayName of this ParamType, to be shown to the user, translated. */
void ParamType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

/*! Returns the index of this \l{ParamType}. The index of an \l{ParamType} indicates the order in the corresponding Type. */
int ParamType::index() const
{
    return m_index;
}

/*! Set the \a index of this \l{ParamType}. */
void ParamType::setIndex(const int &index)
{
    m_index = index;
}

/*! Returns the type of this ParamType. */
QMetaType::Type ParamType::type() const
{
    return m_type;
}

/*! Sets the type of this ParamType to the given \a type. */
void ParamType::setType(QMetaType::Type type)
{
    m_type = type;
}

/*! Returns the default value of this ParamType. */
QVariant ParamType::defaultValue() const
{
    return m_defaultValue;
}

/*! Sets the default value of this ParamType to the given \a defaultValue. */
void ParamType::setDefaultValue(const QVariant &defaultValue)
{
    m_defaultValue = defaultValue;
}

/*! Returns the minimum value of this ParamType. */
QVariant ParamType::minValue() const
{
    return m_minValue;
}

/*! Sets the minimum value of this ParamType to the given \a minValue. */
void ParamType::setMinValue(const QVariant &minValue)
{
    m_minValue = minValue;
}

/*! Returns the maximum value of this ParamType. */
QVariant ParamType::maxValue() const
{
    return m_maxValue;
}

/*! Sets the maximum value of this ParamType to the given \a maxValue. */
void ParamType::setMaxValue(const QVariant &maxValue)
{
    m_maxValue = maxValue;
}

/*! Returns the input type of this ParamType. */
Types::InputType ParamType::inputType() const
{
    return m_inputType;
}

/*! Sets the input value of this ParamType to the given \a inputType. */
void ParamType::setInputType(const Types::InputType &inputType)
{
    m_inputType = inputType;
}

/*! Returns the unit of this ParamType. */
Types::Unit ParamType::unit() const
{
    return m_unit;
}

/*! Sets the unit of this ParamType to the given \a unit. */
void ParamType::setUnit(const Types::Unit &unit)
{
    m_unit = unit;
}

/*! Returns the limits of this ParamType. limits(minValue, maxValue). */
QPair<QVariant, QVariant> ParamType::limits() const
{
    return QPair<QVariant, QVariant>(m_minValue, m_maxValue);
}

/*! Sets the limits of this ParamType. limits(\a min, \a max). */
void ParamType::setLimits(const QVariant &min, const QVariant &max)
{
    m_minValue = min;
    m_maxValue = max;
}

/*! Returns the list of the allowed values of this ParamType. */
QList<QVariant> ParamType::allowedValues() const
{
    return m_allowedValues;
}

/*! Sets the list of the allowed values of this ParamType to the given List of \a allowedValues. */
void ParamType::setAllowedValues(const QList<QVariant> &allowedValues)
{
    m_allowedValues = allowedValues;
}

/*! Returns false if this ParamType is writable by the user. By default a ParamType is always writable. */
bool ParamType::readOnly() const
{
    return m_readOnly;
}

/*! Sets this ParamType \a readOnly. By default a ParamType is always writable. */
void ParamType::setReadOnly(const bool &readOnly)
{
    m_readOnly = readOnly;
}

/*! Returns true if this ParamType is valid. A ParamType is valid, if the id, the name and the data type is set. */
bool ParamType::isValid() const
{
    return !m_id.isNull() && !m_name.isEmpty() && m_type != QMetaType::UnknownType;
}

/*! Returns a list of all valid JSON properties a ParamType JSON definition can have. */
QStringList ParamType::typeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "type" << "defaultValue" << "inputType"
                         << "unit" << "minValue" << "maxValue" << "allowedValues" << "readOnly";
}

/*! Returns a list of mandatory JSON properties a ParamType JSON definition must have. */
QStringList ParamType::mandatoryTypeProperties()
{
    return QStringList() << "id" << "name" << "displayName" << "type";
}

/*! Writes the name, type, defaultValue, min value, max value and readOnly of the given \a paramType to \a dbg. */
QDebug operator<<(QDebug dbg, const ParamType &paramType)
{
    QString typeName;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    typeName = QString(QMetaType(paramType.type()).name());
#else
    typeName = QVariant::typeToName(paramType.type());
#endif

    QDebugStateSaver saver(dbg);
    dbg.nospace() << "ParamType(Id" << paramType.id().toString()
                  << "  Name: " << paramType.name()
                  << ", Type:" << typeName
                  << ", Default:" << paramType.defaultValue()
                  << ", Min:" << paramType.minValue()
                  << ", Max:" << paramType.maxValue()
                  << ", Allowed values:" << paramType.allowedValues()
                  << ", ReadOnly:" << paramType.readOnly()
                  << ")";

    return dbg;
}

/*! Writes the name, type defaultValue, min and max value of each \a paramTypes to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<ParamType> &paramTypes)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "ParamTypeList (count:" << paramTypes.count() << ")" << '\n';
    for (int i = 0; i < paramTypes.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << paramTypes.at(i) << '\n';
    }

    return dbg;
}

ParamTypes::ParamTypes(const QList<ParamType> &other): QList<ParamType>(other)
{
}

bool ParamTypes::contains(const ParamTypeId &paramTypeId)
{
    foreach (const ParamType &paramType, *this) {
        if (paramType.id() == paramTypeId) {
            return true;
        }
    }
    return false;
}

bool ParamTypes::contains(const QString &name)
{
    foreach (const ParamType &paramType, *this) {
        if (paramType.name() == name) {
            return true;
        }
    }
    return false;
}

QVariant ParamTypes::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ParamTypes::put(const QVariant &variant)
{
    append(variant.value<ParamType>());
}

ParamType ParamTypes::findByName(const QString &name) const
{
    foreach (const ParamType &paramType, *this) {
        if (paramType.name() == name) {
            return paramType;
        }
    }
    return ParamType();
}

ParamType ParamTypes::findById(const ParamTypeId &id) const
{
    foreach (const ParamType &paramType, *this) {
        if (paramType.id() == id) {
            return paramType;
        }
    }
    return ParamType();
}

ParamType &ParamTypes::operator[](const QString &name)
{
    int index = -1;
    for (int i = 0; i < count(); i++) {
        if (at(i).name() == name) {
            index = i;
            break;
        }
    }
    return QList::operator[](index);
}
