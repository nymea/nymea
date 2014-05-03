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

#ifndef PARAMDESCRIPTOR_H
#define PARAMDESCRIPTOR_H

#include "param.h"

class ParamDescriptor : public Param
{
public:
    enum OperandType {
        OperandTypeEquals,
        OperandTypeNotEquals,
        OperandTypeLess,
        OperandTypeGreater,
        OperandTypeLessOrEqual,
        OperandTypeGreaterOrEqual
    };
    ParamDescriptor(const QString &name, const QVariant &value = QVariant());

    OperandType operand() const;
    void setOperand(OperandType operand);

private:
    OperandType m_operand;
};

#endif // PARAMDESCRIPTOR_H
