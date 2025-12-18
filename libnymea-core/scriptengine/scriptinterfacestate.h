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

#ifndef SCRIPTINTERFACESTATE_H
#define SCRIPTINTERFACESTATE_H

#include <QObject>
#include <QQmlParserStatus>
#include <QUuid>

#include "integrations/thingmanager.h"
#include "types/state.h"

namespace nymeaserver {
namespace scriptengine {

class ScriptParams;

class ScriptInterfaceState : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString stateName READ stateName WRITE setStateName NOTIFY stateNameChanged)
public:
    ScriptInterfaceState(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString interfaceName() const;
    void setInterfaceName(const QString &interfaceName);

    QString stateName() const;
    void setStateName(const QString &stateName);

private slots:
    void onStateChanged(Thing *thing, const StateTypeId &stateTypeId, const QVariant &value);

signals:
    void interfaceNameChanged();
    void stateNameChanged();

    void stateChanged(const QString &thingId, const QVariant &value);

private:
    ThingManager *m_thingManager = nullptr;

    QString m_interfaceName;
    QString m_stateName;
};

} // namespace scriptengine
} // namespace nymeaserver

#endif // SCRIPTINTERFACESTATE_H
