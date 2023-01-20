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

#ifndef SCRIPTINTERFACESTATE_H
#define SCRIPTINTERFACESTATE_H

#include <QObject>
#include <QUuid>
#include <QQmlParserStatus>

#include "types/state.h"
#include "integrations/thingmanager.h"

namespace nymeaserver {
namespace scriptengine {

class ScriptParams;

class ScriptInterfaceState: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString stateName READ stateName WRITE setStateName NOTIFY stateNameChanged)
public:
    ScriptInterfaceState(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString interfaceName() const;
    void setInterfaceName(const QString &interfaceName);

    QString stateName() const;
    void setStateName(const QString &stateName);

private slots:
    void onStateChanged(Thing *thing, const QString &stateName, const QVariant &value);

signals:
    void interfaceNameChanged();
    void stateNameChanged();

    void stateChanged(const QString &thingId, const QVariant &value);

private:
    ThingManager *m_thingManager = nullptr;

    QString m_interfaceName;
    QString m_stateName;
};

}
}

#endif // SCRIPTINTERFACESTATE_H
