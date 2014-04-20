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

#ifndef ACTION_H
#define ACTION_H

#include "typeutils.h"
#include "param.h"

#include <QVariantList>

class Action
{
public:
    explicit Action(const DeviceId &deviceId, const ActionTypeId &actionTypeId);

    bool isValid() const;

    ActionTypeId actionTypeId() const;
    DeviceId deviceId() const;

    QList<Param> params() const;
    void setParams(const QList<Param> &params);
    Param param(const QString &paramName) const;

private:
    ActionTypeId m_actionTypeId;
    DeviceId m_deviceId;
    QList<Param> m_params;
};

#endif // ACTION_H
