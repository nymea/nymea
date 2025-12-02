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

#ifndef PLATFORMSYSTEMCONTROLLER_H
#define PLATFORMSYSTEMCONTROLLER_H

#include <QObject>
#include <QTimeZone>

class PlatformSystemController : public QObject
{
    Q_OBJECT
public:
    explicit PlatformSystemController(QObject *parent = nullptr);
    virtual ~PlatformSystemController() = default;

    virtual bool powerManagementAvailable() const;
    virtual bool restart();
    virtual bool reboot();
    virtual bool shutdown();

    virtual bool timeManagementAvailable() const;
    virtual bool automaticTimeAvailable() const;
    virtual bool automaticTime() const;
    virtual bool setTime(const QDateTime &time);
    virtual bool setAutomaticTime(bool automaticTime);
    virtual bool setTimeZone(const QTimeZone &timeZone);

    virtual QString deviceSerialNumber() const;


signals:
    void availableChanged();
    void timeZoneManagementAvailableChanged();

    void timeConfigurationChanged();
};

Q_DECLARE_INTERFACE(PlatformSystemController, "io.nymea.PlatformSystemController")

#endif // PLATFORMSYSTEMCONTROLLER_H
