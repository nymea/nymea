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

/*!
    \class GuhSettings
    \brief The settings class for guh.

    \ingroup devices
    \inmodule libguh

    Depending on how the guh server was started (which user started guhd), the setting have to
    be stored in different locations. This class represents a centralized mechanism to store
    settings of the system. The different settings are represented ba the \l{SettingsRole} and
    can be used everywhere in the project.

*/

/*! \enum GuhSettings::SettingsRole
    Represents the role for the \l{GuhSettings}. Each role creates its own settings file.

    \value SettingsRoleNone
        No role will be used. This sould not be used!
    \value SettingsRoleDevices
        This role will create the \b{devices.conf} file and is used to store the configured \l{Device}{Devices}.
    \value SettingsRoleRules
        This role will create the \b{rules.conf} file and is used to store the configured \l{guhserver::Rule}{Rules}.
    \value SettingsRolePlugins
        This role will create the \b{plugins.conf} file and is used to store the \l{DevicePlugin}{Plugin} configurations.
    \value SettingsRoleGlobal
        This role will create the \b{guhd.conf} file and is used to store the global settings of the guh system. This settings
        file is read only.
*/

#include "guhsettings.h"
#include "unistd.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

/*! Constructs a \l{GuhSettings} instance with the given \a role and \a parent. */
GuhSettings::GuhSettings(const SettingsRole &role, QObject *parent):
    QObject(parent),
    m_role(role)
{
    QString settingsFile;
    QString settingsPrefix = QCoreApplication::instance()->organizationName();
    bool rootPrivilege = isRoot();

    switch (role) {
    case SettingsRoleNone:
        break;
    case SettingsRoleDevices:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = "/tmp/" + settingsPrefix + "/test-devices.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created test-devices settings" << m_settings->fileName();
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/devices.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
            qDebug() << "Created device settings" << m_settings->fileName();
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/devices.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created device settings" << m_settings->fileName();
        }
        break;
    case SettingsRoleRules:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = "/tmp/" + settingsPrefix + "/test-rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created test-rules settings" << m_settings->fileName();
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
            qDebug() << "Created rule settings" << m_settings->fileName();
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created rule settings" << m_settings->fileName();
        }
        break;
    case SettingsRolePlugins:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = "/tmp/" + settingsPrefix + "/test-plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created test-plugins settings" << m_settings->fileName();
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
            qDebug() << "Created plugin settings" << m_settings->fileName();
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created plugin settings" << m_settings->fileName();
        }
        break;
    case SettingsRoleGlobal:
        // this file schould always be readable and should never be written
        settingsFile = "/etc/guh/guhd.conf";
        m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        qDebug() << "Created test guhd settings" << m_settings->fileName();
        break;
    default:
        break;
    }
}

/*! Destructor of the GuhSettings.*/
GuhSettings::~GuhSettings()
{
    m_settings->sync();
    delete m_settings;
}

/*! Returns the \l{SettingsRole} of this \l{GuhSettings}.*/
GuhSettings::SettingsRole GuhSettings::settingsRole() const
{
    return m_role;
}

/*! Returns true if guhd is started as \b{root}.*/
bool GuhSettings::isRoot()
{
    if (getuid() != 0)
        return false;

    return true;
}

/*! Returns the path where the logging database will be stored.

  \sa guhserver::LogEngine
*/
QString GuhSettings::logPath()
{
    QString logPath;
    QString organisationName = QCoreApplication::instance()->organizationName();

    if (organisationName == "guh-test") {
        logPath = "/tmp/" + organisationName + "/guhd-test.sqlite";
    } else if (GuhSettings::isRoot()) {
        logPath = "/var/log/guhd.sqlite";
    } else {
        logPath = QDir::homePath() + "/.config/" + organisationName + "/guhd.sqlite";
    }

    return logPath;
}

/*! Returns the path where the log file (console log) will be stored. */
QString GuhSettings::consoleLogPath()
{
    QString consoleLogPath;
    QString organisationName = QCoreApplication::instance()->organizationName();

    if (organisationName == "guh-test") {
        consoleLogPath = "/tmp/" + organisationName + "/guhd-test.log";
    } else if (GuhSettings::isRoot()) {
        consoleLogPath = "/var/log/guhd.log";
    } else {
        consoleLogPath = QDir::homePath() + "/.config/" + organisationName + "/guhd.log";
    }

    return consoleLogPath;
}

/*! Return a list of all settings keys.*/
QStringList GuhSettings::allKeys() const
{
    return m_settings->allKeys();
}

/*! Begins a new group with the given \a prefix.*/
void GuhSettings::beginGroup(const QString &prefix)
{
    m_settings->beginGroup(prefix);
}

/*! Returns a list of all key top-level groups that contain keys that can be read
 *  using the \l{GuhSettings} object.*/
QStringList GuhSettings::childGroups() const
{
    return m_settings->childGroups();
}

/*! Returns a list of all top-level keys that can be read using the \l{GuhSettings} object.*/
QStringList GuhSettings::childKeys() const
{
    return m_settings->childKeys();
}

/*! Removes all entries in the primary location associated to this \l{GuhSettings} object.*/
void GuhSettings::clear()
{
    m_settings->clear();
}

/*! Returns true if there exists a setting called \a key; returns false otherwise. */
bool GuhSettings::contains(const QString &key) const
{
    return m_settings->contains(key);
}

/*! Resets the group to what it was before the corresponding beginGroup() call. */
void GuhSettings::endGroup()
{
    m_settings->endGroup();
}

/*! Returns the current group. */
QString GuhSettings::group() const
{
    return m_settings->group();
}

/*! Returns the path where settings written using this \l{GuhSettings} object are stored. */
QString GuhSettings::fileName() const
{
    return m_settings->fileName();
}

/*! Returns true if settings can be written using this \l{GuhSettings} object; returns false otherwise. */
bool GuhSettings::isWritable() const
{
    return m_settings->isWritable();
}

/*! Removes the setting key and any sub-settings of \a key. */
void GuhSettings::remove(const QString &key)
{
    m_settings->remove(key);
}

/*! Sets the \a value of setting \a key to value. If the \a key already exists, the previous value is overwritten. */
void GuhSettings::setValue(const QString &key, const QVariant &value)
{
    //Q_ASSERT_X(m_role != GuhSettings::SettingsRoleGlobal, "GuhSettings", "Bad settings usage. The global settings file should be read only.");
    m_settings->setValue(key, value);
}

/*! Returns the value for setting \a key. If the setting doesn't exist, returns \a defaultValue. */
QVariant GuhSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

