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

#ifndef SCRIPTINTERFACEACTION_H
#define SCRIPTINTERFACEACTION_H

#include <QObject>
#include <QQmlParserStatus>

class ThingManager;

namespace nymeaserver {
namespace scriptengine {

class ScriptInterfaceAction : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName NOTIFY interfaceNameChanged)
    Q_PROPERTY(QString actionName READ actionName WRITE setActionName NOTIFY actionNameChanged)
public:
    explicit ScriptInterfaceAction(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString interfaceName() const;
    void setInterfaceName(const QString &interfaceName);

    QString actionName() const;
    void setActionName(const QString &actionName);

public slots:
    void execute(const QVariantMap &params);

signals:
    void interfaceNameChanged();
    void actionNameChanged();

public:
    ThingManager *m_thingManager = nullptr;
    QString m_interfaceName;
    QString m_actionName;
};

}
}

#endif // SCRIPTINTERFACEACTION_H
