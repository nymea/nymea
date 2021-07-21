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

#ifndef SCRIPTINTERFACEEVENT_H
#define SCRIPTINTERFACEEVENT_H

#ifdef WITH_QML

#include <QObject>
#include <QUuid>
#include <QQmlParserStatus>

#include "types/event.h"
#include "integrations/thingmanager.h"

namespace nymeaserver {

class ScriptParams;

class ScriptInterfaceEvent: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString eventName READ eventName WRITE setEventName NOTIFY eventNameChanged)
public:
    ScriptInterfaceEvent(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString interfaceName() const;
    void setInterfaceName(const QString &interfaceName);

    QString eventName() const;
    void setEventName(const QString &eventName);

private slots:
    void onEventTriggered(const Event &event);

signals:
    void interfaceNameChanged();
    void eventNameChanged();

    void triggered(const QString &thingId, const QVariantMap &params);

private:
    ThingManager *m_thingManager = nullptr;

    QString m_interfaceName;
    QString m_eventName;
};

}

#endif // WITH_QML

#endif // SCRIPTINTERFACEEVENT_H
