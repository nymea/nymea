/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
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


#ifndef STATETYPE_H
#define STATETYPE_H

#include "libguh.h"
#include "typeutils.h"

#include <QVariant>

class LIBGUH_EXPORT StateType
{
public:
    StateType(const StateTypeId &id);

    StateTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    int index() const;
    void setIndex(const int &index);

    QVariant::Type type() const;
    void setType(const QVariant::Type &type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

    QVariantList possibleValues() const;
    void setPossibleValues(const QVariantList &possibleValues);

    Types::Unit unit() const;
    void setUnit(const Types::Unit &unit);

    bool ruleRelevant() const;
    void setRuleRelevant(const bool &ruleRelevant);

    bool graphRelevant() const;
    void setGraphRelevant(const bool &graphRelevant);

private:
    StateTypeId m_id;
    QString m_name;
    int m_index;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    QVariantList m_possibleValues;
    Types::Unit m_unit;
    bool m_ruleRelevant;
    bool m_graphRelevant;

};

class StateTypes: public QList<StateType>
{
public:
    StateTypes(const QList<StateType> &other);
    StateType findByName(const QString &name);
    StateType findById(const StateTypeId &id);
};

#endif // STATETYPE_H
