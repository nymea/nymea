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

#ifndef SCRIPTTHING_H
#define SCRIPTTHING_H

#include "integrations/thingmanager.h"
#include <QObject>
#include <QQmlParserStatus>
#include <QUuid>

namespace nymeaserver {
namespace scriptengine {

class ScriptThing : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString thingId READ thingId WRITE setThingId NOTIFY thingIdChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

public:
    Q_ENUM(Thing::ThingError)

    explicit ScriptThing(QObject *parent = nullptr);
    explicit ScriptThing(ThingManager *thingManager, QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString thingId() const;
    void setThingId(const QString &thingId);

    QString name() const;

    Q_INVOKABLE QVariant stateValue(const QString &stateName) const;

    Q_INVOKABLE void setStateValue(const QString &stateName, const QVariant &value);
    Q_INVOKABLE void executeAction(const QString &actionName, const QVariantMap &params);

signals:
    void thingIdChanged();
    void nameChanged();

    void stateValueChanged(const QString &stateName, const QVariant &value);
    void eventTriggered(const QString &eventName, const QVariantMap &params);
    void actionExecuted(const QString &actionName, const QVariantMap &params, Thing::ThingError status, Action::TriggeredBy triggeredBy);

private slots:
    void init(ThingManager *thingManager);
    void connectToThing();

private:
    ThingId m_thingId;
    ThingManager *m_thingManager = nullptr;

    QMetaObject::Connection m_nameConnection;
};

} // namespace scriptengine
} // namespace nymeaserver

#endif // SCRIPTTHING_H
