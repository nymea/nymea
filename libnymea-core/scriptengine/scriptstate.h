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

#ifndef SCRIPTSTATE_H
#define SCRIPTSTATE_H

#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>

#include "integrations/thingactioninfo.h"
#include "integrations/thingmanager.h"

class Logger;

namespace nymeaserver {
namespace scriptengine {

class ScriptState : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString thingId READ thingId WRITE setThingId NOTIFY thingIdChanged)
    Q_PROPERTY(QString deviceId READ thingId WRITE setThingId NOTIFY thingIdChanged) // DEPRECATED
    Q_PROPERTY(QString stateTypeId READ stateTypeId WRITE setStateTypeId NOTIFY stateTypeChanged)
    Q_PROPERTY(QString stateName READ stateName WRITE setStateName NOTIFY stateTypeChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant minimumValue READ minimumValue NOTIFY stateTypeChanged)
    Q_PROPERTY(QVariant maximumValue READ maximumValue NOTIFY stateTypeChanged)

public:
    explicit ScriptState(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString thingId() const;
    void setThingId(const QString &thingId);

    QString stateTypeId() const;
    void setStateTypeId(const QString &stateTypeId);

    QString stateName() const;
    void setStateName(const QString &stateName);

    QVariant value() const;
    void setValue(const QVariant &value);

    QVariant minimumValue() const;
    QVariant maximumValue() const;

public slots:
    void store();
    void restore();

signals:
    void thingIdChanged();
    void stateTypeChanged();
    void valueChanged();

private slots:
    void onThingStateChanged(Thing *thing, const StateTypeId &stateTypeId);

    void connectToThing();

private:
    ThingManager *m_thingManager = nullptr;
    Logger *m_logger = nullptr;
    QUuid m_scriptId;

    QString m_thingId;
    QString m_stateTypeId;
    QString m_stateName;

    ThingActionInfo *m_pendingActionInfo = nullptr;
    QVariant m_valueCache;

    QVariant m_valueStore;

    QMetaObject::Connection m_connection;
};

} // namespace scriptengine
} // namespace nymeaserver

#endif // SCRIPTSTATE_H
