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

#ifndef SCRIPTTHING_H
#define SCRIPTTHING_H

#include <QObject>
#include <QQmlParserStatus>
#include <QUuid>
#include "integrations/thingmanager.h"

namespace nymeaserver {
namespace scriptengine {

class ScriptThing : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString thingId READ thingId WRITE setThingId NOTIFY thingIdChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
public:
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

private slots:
    void init(ThingManager *thingManager);
    void connectToThing();

private:
    ThingId m_thingId;
    ThingManager *m_thingManager = nullptr;

    QMetaObject::Connection m_nameConnection;
};

}
}

#endif // SCRIPTTHING_H
