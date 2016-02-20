/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef GUHSETTINGS_H
#define GUHSETTINGS_H

#include <QObject>
#include <QVariant>

#include "libguh.h"

class QSettings;

class LIBGUH_EXPORT GuhSettings : public QObject
{
    Q_OBJECT
public:
    enum SettingsRole {
        SettingsRoleNone,
        SettingsRoleDevices,
        SettingsRoleRules,
        SettingsRolePlugins,
        SettingsRoleGlobal
    };

    explicit GuhSettings(const SettingsRole &role = SettingsRoleNone, QObject *parent = 0);
    ~GuhSettings();

    SettingsRole settingsRole() const;

    static bool isRoot();
    static QString logPath();
    static QString settingsPath();
    static QString consoleLogPath();

    // forwarded QSettings methods
    QStringList	allKeys() const;
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
    QSettings *m_settings;
    SettingsRole m_role;

};

#endif // GUHSETTINGS_H
