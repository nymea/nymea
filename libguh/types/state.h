/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef STATE_H
#define STATE_H

#include <QUuid>
#include <QVariant>

class State
{
public:
    State(const QUuid &stateTypeId, const QUuid &deviceId);

    QUuid stateTypeId() const;
    QUuid deviceId() const;

    QVariant value() const;
    void setValue(const QVariant &value);

private:
    QUuid m_stateTypeId;
    QUuid m_deviceId;
    QVariant m_value;
};

#endif // STATE_H
