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

#ifndef NYMEASETTINGS_H
#define NYMEASETTINGS_H

#include <QObject>
#include <QVariant>

#include "libnymea.h"

class QSettings;

class LIBNYMEA_EXPORT NymeaSettings : public QObject
{
    Q_OBJECT
public:
    enum SettingsRole {
        SettingsRoleNone,
        SettingsRoleThings,
        SettingsRoleRules,
        SettingsRolePlugins,
        SettingsRoleGlobal,
        SettingsRoleTags,
        SettingsRoleMqttPolicies,
        SettingsRoleIOConnections,
        SettingsRoleZigbee,
        SettingsRoleModbusRtu,
        SettingsRoleZWave
    };
    Q_ENUM(SettingsRole)

    explicit NymeaSettings(const SettingsRole &role = SettingsRoleNone, QObject *parent = nullptr);
    ~NymeaSettings();

    SettingsRole settingsRole() const;

    // Creates the file from defaults if it does not exist yet
    static QString privodeFromDefaultFilePath(const QString &fileName);

    static bool isRoot();

    static QString settingsPath();
    static QString defaultSettingsPath();
    static QString translationsPath();
    static QString scriptsPath();
    static QString storagePath();
    static QString cachePath();

    // Forwarded QSettings methods
    QStringList	allKeys() const;
    void beginWriteArray(const QString &prefix);
    void setArrayIndex(int i);
    int beginReadArray(const QString &prefix);

    void endArray();
    void beginGroup(const QString &prefix);
    QStringList	childGroups() const;
    QStringList	childKeys() const;
    void clear();
    bool contains(const QString &key) const;
    void endGroup();
    QString	group() const;
    QString	fileName() const;
    bool isWritable() const;
    void remove(const QString &key);
    void setValue(const QString & key, const QVariant &value);
    QVariant value(const QString & key, const QVariant & defaultValue = QVariant()) const;

private:
    QSettings *m_settings = nullptr;
    SettingsRole m_role = SettingsRoleNone;

};

#endif // NYMEASETTINGS_H
