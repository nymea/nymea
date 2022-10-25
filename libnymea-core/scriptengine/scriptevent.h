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

#ifndef SCRIPTEVENT_H
#define SCRIPTEVENT_H

#include <QObject>
#include <QUuid>
#include <QQmlParserStatus>

#include "types/event.h"
#include "integrations/thingmanager.h"

namespace nymeaserver {
namespace scriptengine {

class ScriptParams;

class ScriptEvent: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString thingId READ thingId WRITE setThingId NOTIFY thingIdChanged)
    Q_PROPERTY(QString deviceId READ thingId WRITE setThingId NOTIFY thingIdChanged) // DEPRECATED
    Q_PROPERTY(QString eventTypeId READ eventTypeId WRITE setEventTypeId NOTIFY eventTypeIdChanged)
    Q_PROPERTY(QString eventName READ eventName WRITE setEventName NOTIFY eventNameChanged)
public:
    ScriptEvent(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString thingId() const;
    void setThingId(const QString &thingId);

    QString eventTypeId() const;
    void setEventTypeId(const QString &eventTypeId);

    QString eventName() const;
    void setEventName(const QString &eventName);

private slots:
    void onEventTriggered(const Event &event);

signals:
    void thingIdChanged();
    void eventTypeIdChanged();
    void eventNameChanged();

    void triggered(const QVariantMap &params);

private:
    ThingManager *m_thingManager = nullptr;

    QString m_thingId;
    QString m_eventTypeId;
    QString m_eventName;
};

}
}

#endif // SCRIPTEVENT_H
