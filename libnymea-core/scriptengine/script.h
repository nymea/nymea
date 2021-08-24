/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef WITH_QML

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

#endif // WITH_QML

#endif // SCRIPT_H
