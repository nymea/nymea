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

#ifndef SCRIPTTHINGS_H
#define SCRIPTTHINGS_H

#include "integrations/thingmanager.h"

#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QQmlParserStatus>


namespace nymeaserver {
namespace scriptengine {

class ScriptThing;
class ThingsModel;


class ScriptThings : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString filterInterface READ filterInterface WRITE setFilterInterface NOTIFY filterInterfaceChanged)

public:
    explicit ScriptThings(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString filterInterface() const;
    void setFilterInterface(const QString &filterInterface);

    Q_INVOKABLE nymeaserver::scriptengine::ScriptThing *get(int index) const;
    Q_INVOKABLE nymeaserver::scriptengine::ScriptThing *getThing(const QUuid &thingId) const;

signals:
    void countChanged();
    void filterInterfaceChanged();
    void thingAdded(const QString &thingId);
    void thingRemoved(const QString &thingId);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    ThingManager *m_thingManager = nullptr;
    ThingsModel *m_model = nullptr;

    QString m_filterInterface;
};

class ThingsModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        RoleId,
        RoleName
    };
    ThingsModel(ThingManager *thingManager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Thing *get(int index) const;
    Thing *getThing(const QUuid &thingId) const;

signals:
    void countChanged();

private:
    ThingManager *m_thingManager = nullptr;

};

}
}
#endif // SCRIPTTHINGS_H
