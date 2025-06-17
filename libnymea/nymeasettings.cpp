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

/*!
    \class NymeaSettings
    \brief The settings class for nymea.

    \ingroup devices
    \inmodule libnymea

    Depending on how the nymea server was started (which user started nymead), the setting have to
    be stored in different locations. This class represents a centralized mechanism to store
    settings of the system. The different settings are represented ba the \l{SettingsRole} and
    can be used everywhere in the project.

*/

/*! \enum NymeaSettings::SettingsRole
    Represents the role for the \l{NymeaSettings}. Each role creates its own settings file.

    \value SettingsRoleNone
        No role will be used. This sould not be used!
    \value SettingsRoleDevices
        This role will create the \b{things.conf} file and is used to store the configured \l{Device}{Devices}.
    \value SettingsRoleRules
        This role will create the \b{rules.conf} file and is used to store the configured \l{nymeaserver::Rule}{Rules}.
    \value SettingsRolePlugins
        This role will create the \b{plugins.conf} file and is used to store the \l{DevicePlugin}{Plugin} configurations.
    \value SettingsRoleGlobal
        This role will create the \b{nymead.conf} file and is used to store the global settings of the nymea system. This settings
        file is read only.
    \value SettingsRoleDeviceStates
        This role will create the \b{device-states.conf} file and is used to store the configured \l{Device} \l{State}{States}.
    \value SettingsRoleTags
        This role will create the \b{tags.conf} file and is used to store the \l{Tag}{Tags}.
    \value SettingsRoleMqttPolicies
        This role will create the \b{mqttpolicies.conf} file and is used to store the \l{MqttPolicy}{MqttPolicies}.
    \value SettingsRoleIOConnections
        This role will create the \b{ioconnections.conf} file and is used to store the \l{IOConnection}{IOConnections}.
    \value SettingsRoleZigbee
        This role will create the \b{zigbee.conf} file and is used to store the Zigbee networks.
    \value SettingsRoleModbusRtu
        This role will create the \b{modbusrtu.conf} file and is used to store the modbus RTU resources.

*/

#include "nymeasettings.h"
#include "loggingcategories.h"

#include <unistd.h>

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>


/*! Constructs a \l{NymeaSettings} instance with the given \a role and \a parent. */
NymeaSettings::NymeaSettings(const SettingsRole &role, QObject *parent):
    QObject(parent),
    m_role(role)
{
    /*
        Since 1.12.0 we consider the /etc/nymea folder as read only from a deamon perspective.
        The /etc/nymea/ directory should be used for default settings the system is using in order to
        generate the first configuration in /var/lib/nymea/

        It is up to the platform maintainer to provide specific default configurations or just use the defaults.

        1. Check env, if that is specified, we use that as settings storage, yet check if there are any defaults to load
        2. Check if we have the setting file in /var/lib/nymea, if so use that RW
        3. If there no config file in RW, load read only from /etc/nymea/ and save to /var/lib/nymea
    */

    QString fileName;
    switch (role) {
    case SettingsRoleNone:
        break;
    case SettingsRoleThings:
        fileName = "things.conf";
        break;
    case SettingsRoleRules:
        fileName = "rules.conf";
        break;
    case SettingsRolePlugins:
        fileName = "plugins.conf";
        break;
    case SettingsRoleGlobal:
        fileName = "nymead.conf";
        break;
    case SettingsRoleTags:
        fileName = "tags.conf";
        break;
    case SettingsRoleMqttPolicies:
        fileName = "mqttpolicies.conf";
        break;
    case SettingsRoleIOConnections:
        fileName = "ioconnections.conf";
        break;
    case SettingsRoleZigbee:
        fileName = "zigbee.conf";
        break;
    case SettingsRoleModbusRtu:
        fileName = "modbusrtu.conf";
        break;
    case SettingsRoleZWave:
        fileName = "zwave.conf";
        break;
    }

    m_settings = new QSettings(NymeaSettings::privodeFromDefaultFilePath(fileName), QSettings::IniFormat, this);
}

/*! Destructor of the NymeaSettings.*/
NymeaSettings::~NymeaSettings()
{
    m_settings->sync();
    delete m_settings;
}

QString NymeaSettings::privodeFromDefaultFilePath(const QString &filePath)
{
    QFileInfo settingsFileInfo(filePath);
    if (!settingsFileInfo.exists()) {
        // Settings file does not exist yet, check if we have a default version of the file
        QFileInfo defaultSettingsFileInfo(NymeaSettings::defaultSettingsPath() + QDir::separator() + settingsFileInfo.fileName());
        if (defaultSettingsFileInfo.exists()) {
            qCDebug(dcSystem()) << "No configuration file found in" << settingsFileInfo.absolutePath() << "using"
                                << defaultSettingsFileInfo.absoluteFilePath() << "as default settings.";

            QString destinationPath = settingsFileInfo.absolutePath();
            if (!(QDir(destinationPath).exists())) {
                if (!QDir().mkdir(destinationPath)) {
                    qCWarning(dcSystem()) << "Unable to create directory for storing configuration" << destinationPath;
                } else {
                    qCDebug(dcSystem()) << "Created successfully directory for storing configuration" << destinationPath;
                }
            }

            QFile defaultSettingsFile(defaultSettingsFileInfo.absoluteFilePath());
            if (!defaultSettingsFile.copy(settingsFileInfo.absoluteFilePath())) {
                qCWarning(dcSystem()) << "Failed to copy configuration" << defaultSettingsFileInfo.absoluteFilePath() << "to"
                                      << settingsFileInfo.absoluteFilePath() << defaultSettingsFile.errorString();
            } else {
                qCDebug(dcSystem()) << "Copied successfully configuration" << defaultSettingsFileInfo.absoluteFilePath() << "to"
                                    << settingsFileInfo.absoluteFilePath();
            }
        }
    }

    return settingsFileInfo.absoluteFilePath();
}

/*! Returns the \l{SettingsRole} of this \l{NymeaSettings}.*/
NymeaSettings::SettingsRole NymeaSettings::settingsRole() const
{
    return m_role;
}

/*! Returns true if nymead is started as \b{root}.*/
bool NymeaSettings::isRoot()
{
    if (getuid() != 0)
        return false;

    return true;
}

/*! Returns the path to the folder where the NymeaSettings will be saved i.e. \tt{/var/lib/nymea}. */
QString NymeaSettings::settingsPath()
{
    QString settingsPrefix = QCoreApplication::instance()->organizationName();

    QString path;
    if (!qEnvironmentVariableIsEmpty("SNAP")) {
        path = QString(qgetenv("SNAP_DATA"));
    } else if (!qEnvironmentVariableIsEmpty("NYMEA_CONFIG_PATH")) {
        path = QString(qgetenv("NYMEA_CONFIG_PATH"));
    } else if (settingsPrefix.contains("nymea-test")) {
        path = "/tmp/" + settingsPrefix;
    } else if (isRoot()) {
        path = "/var/lib/" + settingsPrefix;
    } else {
        path = QDir::homePath() + "/.config/" + settingsPrefix;
    }

    return QDir(path).absolutePath();
}

/*! Returns the path to the folder where the default NymeaSettings can be loaded i.e. \tt{/etc/nymea}. This location should be considered read only. */
QString NymeaSettings::defaultSettingsPath()
{
    QString organisationName = QCoreApplication::instance()->organizationName();

    QString path;
    if (!qEnvironmentVariableIsEmpty("SNAP")) {
        path = QString(qgetenv("SNAP") + "/etc/" + organisationName);
    } else if (!qEnvironmentVariableIsEmpty("NYMEA_DEFAULT_CONFIG_PATH")) {
        path = QString(qgetenv("NYMEA_DEFAULT_CONFIG_PATH"));
    } else if (organisationName == "nymea-test") {
        path = "/tmp/" + organisationName;
    } else {
        path = "/etc/" + organisationName;
    }

    return QDir(path).absolutePath();
}

/*! Returns the default system translation path \tt{/usr/share/nymea/translations}. */
QString NymeaSettings::translationsPath()
{
    QString organisationName = QCoreApplication::instance()->organizationName();

    QString path;
    if (!qEnvironmentVariableIsEmpty("SNAP")) {
        path =  QString(qgetenv("SNAP") + "/usr/share/nymea/translations");
    } else if (organisationName == "nymea-test") {
        path =  "/tmp/" + organisationName;
    } else {
        path = QString("/usr/share/nymea/translations");
    }

    return QDir(path).absolutePath();
}

QString NymeaSettings::scriptsPath()
{
    return NymeaSettings::storagePath() + "/scripts/";
}

/*! Returns the default system sorage path i.e. \tt{/var/lib/nymea}. */
QString NymeaSettings::storagePath()
{
    // For backwards compatibility
    return NymeaSettings::settingsPath();
}

QString NymeaSettings::cachePath()
{
    QString organisationName = QCoreApplication::instance()->organizationName();

    QString path;
    if (!qEnvironmentVariableIsEmpty("SNAP")) {
        path = QString(qgetenv("SNAP_DATA"));
    } else if (organisationName == "nymea-test") {
        path = "/tmp/" + organisationName + "/cache";
    } else if (NymeaSettings::isRoot()) {
        path = "/var/cache/" + organisationName;
    } else {
        path = QDir::homePath() + "/.cache/" + organisationName;
    }

    return QDir(path).absolutePath();
}

/*! Return a list of all settings keys.*/
QStringList NymeaSettings::allKeys() const
{
    return m_settings->allKeys();
}

/*! Adds \a prefix to the current group and starts writing an array of size size. If size is -1 (the default),
 * it is automatically determined based on the indexes of the entries written. */
void NymeaSettings::beginWriteArray(const QString &prefix)
{
    m_settings->beginWriteArray(prefix);
}

/*! Sets the current array index to \a i. */
void NymeaSettings::setArrayIndex(int i)
{
    m_settings->setArrayIndex(i);
}

/*! Adds \a prefix to the current group and starts reading from an array. Returns the size of the array.*/
int NymeaSettings::beginReadArray(const QString &prefix)
{
    return m_settings->beginReadArray(prefix);
}

/*! End an array. */
void NymeaSettings::endArray()
{
    m_settings->endArray();
}

/*! Begins a new group with the given \a prefix.*/
void NymeaSettings::beginGroup(const QString &prefix)
{
    m_settings->beginGroup(prefix);
}

/*! Returns a list of all key top-level groups that contain keys that can be read
 *  using the \l{NymeaSettings} object.*/
QStringList NymeaSettings::childGroups() const
{
    return m_settings->childGroups();
}

/*! Returns a list of all top-level keys that can be read using the \l{NymeaSettings} object.*/
QStringList NymeaSettings::childKeys() const
{
    return m_settings->childKeys();
}

/*! Removes all entries in the primary location associated to this \l{NymeaSettings} object.*/
void NymeaSettings::clear()
{
    m_settings->clear();
}

/*! Returns true if there exists a setting called \a key; returns false otherwise. */
bool NymeaSettings::contains(const QString &key) const
{
    return m_settings->contains(key);
}

/*! Resets the group to what it was before the corresponding beginGroup() call. */
void NymeaSettings::endGroup()
{
    m_settings->endGroup();
}

/*! Returns the current group. */
QString NymeaSettings::group() const
{
    return m_settings->group();
}

/*! Returns the path where settings written using this \l{NymeaSettings} object are stored. */
QString NymeaSettings::fileName() const
{
    return m_settings->fileName();
}

/*! Returns true if settings can be written using this \l{NymeaSettings} object; returns false otherwise. */
bool NymeaSettings::isWritable() const
{
    return m_settings->isWritable();
}

/*! Removes the setting key and any sub-settings of \a key. */
void NymeaSettings::remove(const QString &key)
{
    m_settings->remove(key);
}

/*! Sets the \a value of setting \a key to value. If the \a key already exists, the previous value is overwritten. */
void NymeaSettings::setValue(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

/*! Returns the value for setting \a key. If the setting doesn't exist, returns \a defaultValue. */
QVariant NymeaSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}
