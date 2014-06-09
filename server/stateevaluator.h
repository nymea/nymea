/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef STATEEVALUATOR_H
#define STATEEVALUATOR_H

#include "types/state.h"
#include "types/statedescriptor.h"

#include <QSettings>

class StateEvaluator
{
public:
    enum OperatorType {
        OperatorTypeAnd,
        OperatorTypeOr
    };

    StateEvaluator();
    StateEvaluator(const StateEvaluatorId &id, const StateDescriptor &statedescriptor);
    StateEvaluator(const StateEvaluatorId &id, QList<StateEvaluator> childEvaluators = QList<StateEvaluator>(), StateOperator stateOperator = StateOperatorAnd);

    StateEvaluatorId id() const;

    StateDescriptor stateDescriptor() const;

    QList<StateEvaluator> childEvaluators() const;
    void setChildEvaluators(const QList<StateEvaluator> &childEvaluators);
    void appendEvaluator(const StateEvaluator &stateEvaluator);

    StateOperator operatorType() const;
    void setOperatorType(StateOperator operatorType);

    bool evaluate() const;

    void dumpToSettings(QSettings &settings, const QString &groupName) const;
    static StateEvaluator loadFromSettings(QSettings &settings, const QString &groupPrefix);

private:
    StateEvaluatorId m_id;
    StateDescriptor m_stateDescriptor;

    QList<StateEvaluator> m_childEvaluators;
    StateOperator m_operatorType;

};

#endif // STATEEVALUATOR_H
