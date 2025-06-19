/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2025, nymea GmbH
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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
