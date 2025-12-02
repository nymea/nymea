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

#ifndef SCRIPTACTION_H
#define SCRIPTACTION_H

#include <QObject>
#include <QQmlParserStatus>
#include <QVariantMap>
#include <QUuid>

#include "integrations/thing.h"
#include "types/action.h"

class Logger;
class ThingManager;

namespace nymeaserver {
namespace scriptengine {

class ScriptAction : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString thingId READ thingId WRITE setThingId NOTIFY thingIdChanged)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString deviceId READ thingId WRITE setThingId NOTIFY thingIdChanged) // DEPRECATED
    Q_PROPERTY(QString actionTypeId READ actionTypeId WRITE setActionTypeId NOTIFY actionTypeIdChanged)
    Q_PROPERTY(QString actionName READ actionName WRITE setActionName NOTIFY actionNameChanged)
public:
    explicit ScriptAction(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString thingId() const;
    void setThingId(const QString &thingId);

    QString interfaceName() const;
    void setInterfaceName(const QString &interfaceName);

    QString actionTypeId() const;
    void setActionTypeId(const QString &actionTypeId);

    QString actionName() const;
    void setActionName(const QString &actionName);

public slots:
    void execute(const QVariantMap &params = QVariantMap());

signals:
    void thingIdChanged();
    void interfaceNameChanged();
    void actionTypeIdChanged();
    void actionNameChanged();

    void executed(const QVariantMap &params, Thing::ThingError status, Action::TriggeredBy triggeredBy);

public:
    ThingManager *m_thingManager = nullptr;
    Logger *m_logger = nullptr;
    QUuid m_scriptId;

    QString m_thingId;
    QString m_interfaceName;
    QString m_actionTypeId;
    QString m_actionName;
};

}
}

#endif // SCRIPTACTION_H
