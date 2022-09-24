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

#ifndef SCRIPTSTATE_H
#define SCRIPTSTATE_H

#include <QObject>
#include <QQmlParserStatus>
#include <QPointer>

#include "integrations/thingmanager.h"
#include "integrations/thingactioninfo.h"

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

    QString m_thingId;
    QString m_stateTypeId;
    QString m_stateName;


    ThingActionInfo *m_pendingActionInfo = nullptr;
    QVariant m_valueCache;

    QVariant m_valueStore;

    QMetaObject::Connection m_connection;
};

}
}

#endif // SCRIPTSTATE_H
