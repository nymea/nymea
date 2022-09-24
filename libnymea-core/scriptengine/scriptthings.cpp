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

#include "scriptthings.h"
#include "scriptthing.h"

#include <qqml.h>
#include <QQmlEngine>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcScriptEngine)

namespace nymeaserver {
namespace scriptengine {

ScriptThings::ScriptThings(QObject *parent)
    : QSortFilterProxyModel{parent}
{

}

void ScriptThings::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());
    m_model = new ThingsModel(m_thingManager, this);
    setSourceModel(m_model);

    connect(m_thingManager, &ThingManager::thingAdded, this, [this](Thing *newThing){
        emit thingAdded(newThing->id().toString());
        emit countChanged();
    });
    connect(m_thingManager, &ThingManager::thingRemoved, this, [this](const ThingId &thingId){
        emit thingRemoved(thingId.toString());
        emit countChanged();
    });
}

void ScriptThings::componentComplete()
{

}

QString ScriptThings::filterInterface() const
{
    return m_filterInterface;
}

void ScriptThings::setFilterInterface(const QString &filterInterface)
{
    if (m_filterInterface != filterInterface) {
        m_filterInterface = filterInterface;
        emit filterInterfaceChanged();
        invalidateFilter();
    }
}

ScriptThing *ScriptThings::get(int index) const
{
    Thing *thing = m_model->get(mapToSource(this->index(index, 0)).row());
    if (!thing) {
        return nullptr;
    }
    ScriptThing *scriptThing = new ScriptThing(m_thingManager);
    QQmlEngine::setObjectOwnership(scriptThing, QQmlEngine::JavaScriptOwnership);
    scriptThing->setThingId(thing->id().toString());
    return scriptThing;

}

ScriptThing *ScriptThings::getThing(const QUuid &thingId) const
{
    Thing *thing = m_model->getThing(thingId);
    if (!thing) {
        return nullptr;
    }
    ScriptThing *scriptThing = new ScriptThing(m_thingManager);
    QQmlEngine::setObjectOwnership(scriptThing, QQmlEngine::JavaScriptOwnership);
    scriptThing->setThingId(thing->id().toString());
    return scriptThing;
}

bool ScriptThings::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)
    Thing *thing = m_model->get(sourceRow);
    if (!m_filterInterface.isEmpty() && !thing->thingClass().interfaces().contains(m_filterInterface)) {
        return false;
    }

    return true;
}

ThingsModel::ThingsModel(ThingManager *thingManager, QObject *parent):
    QAbstractListModel(parent),
    m_thingManager(thingManager)
{
}

int ThingsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_thingManager->configuredThings().count();
}

QVariant ThingsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case RoleId:
        return m_thingManager->configuredThings().at(index.row())->id();
    case RoleName:
        return m_thingManager->configuredThings().at(index.row())->name();
    }
    return QVariant();
}

QHash<int, QByteArray> ThingsModel::roleNames() const
{
    return {
        {RoleId, "thingId"},
        {RoleName, "thingName"}
    };
}

Thing *ThingsModel::get(int index) const
{
    return m_thingManager->configuredThings().at(index);
}

Thing *ThingsModel::getThing(const QUuid &thingId) const
{
    return m_thingManager->findConfiguredThing(thingId);
}

}
}
