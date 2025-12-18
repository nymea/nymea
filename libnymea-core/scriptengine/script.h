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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QMetaObject>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QUuid>

namespace nymeaserver {
namespace scriptengine {

class Script
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
public:
    Script();

    QUuid id() const;
    void setId(const QUuid &id);

    QString name() const;
    void setName(const QString &name);

    QStringList errors;

private:
    QUuid m_id;
    QString m_name;

    friend class ScriptEngine;
    QQmlContext *context = nullptr;
    QQmlComponent *component = nullptr;
    QObject *object = nullptr;
};

class Scripts : public QList<Script>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Scripts();
    Scripts(const QList<Script> &other);
    Q_INVOKABLE QVariant get(int index);
    Q_INVOKABLE void put(const QVariant &value);
};
} // namespace scriptengine
} // namespace nymeaserver
Q_DECLARE_METATYPE(nymeaserver::scriptengine::Script)
Q_DECLARE_METATYPE(nymeaserver::scriptengine::Scripts)

#endif // SCRIPT_H
