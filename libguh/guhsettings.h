/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2017 Simon Stürz <simon.stuerz@guh.io>              *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
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
    static QString translationsPath();
    static QString storagePath();

    // forwarded QSettings methods
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
    QSettings *m_settings;
    SettingsRole m_role;

};

#endif // GUHSETTINGS_H
