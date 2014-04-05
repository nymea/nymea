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

#ifndef DEVICE_H
#define DEVICE_H

#include "types/state.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

class Device: public QObject
{
    Q_OBJECT

    friend class DeviceManager;

public:
    QUuid id() const;
    QUuid deviceClassId() const;
    QUuid pluginId() const;

    QString name() const;
    void setName(const QString &name);

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

    QList<State> states() const;
    void setStates(const QList<State> &states);

    bool hasState(const QUuid &stateTypeId) const;
    QVariant stateValue(const QUuid &stateTypeId) const;
    void setStateValue(const QUuid &stateTypeId, const QVariant &value);

signals:
    void stateValueChanged(const QUuid &stateTypeId, const QVariant &value);

private:
    Device(const QUuid &pluginId, const QUuid &id, const QUuid &deviceClassId, QObject *parent = 0);
    Device(const QUuid &pluginId, const QUuid &deviceClassId, QObject *parent = 0);

private:
    QUuid m_id;
    QUuid m_deviceClassId;
    QUuid m_pluginId;
    QString m_name;
    QVariantMap m_params;
    QList<State> m_states;
};

#endif
