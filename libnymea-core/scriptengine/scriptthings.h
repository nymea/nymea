/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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
