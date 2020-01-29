/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QMetaObject>
#include <QUuid>
#include <QQmlContext>
#include <QQmlComponent>
#include <QObject>

namespace nymeaserver {

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

class Scripts: public QList<Script>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Scripts();
    Scripts(const QList<Script> &other);
    Q_INVOKABLE QVariant get(int index);
    Q_INVOKABLE void put(const QVariant &value);
};

}
Q_DECLARE_METATYPE(nymeaserver::Script)
Q_DECLARE_METATYPE(nymeaserver::Scripts)

#endif // SCRIPT_H
