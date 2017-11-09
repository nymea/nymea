/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PUSHBUTTONAGENT_H
#define PUSHBUTTONAGENT_H

#include <QObject>
#include <QDBusConnection>

class PushButtonAgent : public QObject
{
    Q_OBJECT
public:
    explicit PushButtonAgent(QObject *parent = nullptr);

    bool init(QDBusConnection::BusType busType = QDBusConnection::SystemBus);

signals:
    Q_SCRIPTABLE void PushButtonPressed();

public slots:
    void sendButtonPressed();
};

#endif // PUSHBUTTONAGENT_H
