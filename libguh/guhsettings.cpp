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
#ifdef SNAPPY
    QString settingsFilePath = QString(qgetenv("SNAP_DATA"));
    switch (role) {
    case SettingsRoleNone:
        break;
    case SettingsRoleDevices:
        settingsFile = settingsFilePath + "/devices.conf";
        m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        break;
    case SettingsRoleRules:
        settingsFile = settingsFilePath + "/rules.conf";
        m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        break;
    case SettingsRolePlugins:
        settingsFile = settingsFilePath + "/plugins.conf";
        m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        break;
    case SettingsRoleGlobal:
        settingsFile = settingsFilePath + "/guhd.conf";
        m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        break;
    default:
        break;
    }
#else
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
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/devices.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/devices.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        }
        break;
    case SettingsRoleRules:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = "/tmp/" + settingsPrefix + "/test-rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        }
        break;
    case SettingsRolePlugins:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = "/tmp/" + settingsPrefix + "/test-plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        }
        break;
    case SettingsRoleGlobal:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = "/tmp/" + settingsPrefix + "/test-guhd.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/guhd.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
        } else {
            settingsFile = QDir::homePath() + "/.config/" + settingsPrefix + "/guhd.conf";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
        }
        break;
    default:
        break;
    }

#endif // SNAPPY

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
#ifdef SNAPPY
    logPath = QString(qgetenv("SNAP_COMMON")) + "/guhd.sqlite";
#else
    QString organisationName = QCoreApplication::instance()->organizationName();

    if (organisationName == "guh-test") {
        logPath = "/tmp/" + organisationName + "/guhd-test.sqlite";
    } else if (GuhSettings::isRoot()) {
        logPath = "/var/log/guhd.sqlite";
    } else {
        logPath = QDir::homePath() + "/.config/" + organisationName + "/guhd.sqlite";
    }

#endif // SNAPPY
    return logPath;
}

/*! Returns the path to the folder where the GuhSettings will be saved. */
QString GuhSettings::settingsPath()
{
    QString path;
#ifdef SNAPPY
    path = QString(qgetenv("SNAP_DATA"));
#else
    QString organisationName = QCoreApplication::instance()->organizationName();

    if (organisationName == "guh-test") {
        path = "/tmp/" + organisationName;
    } else if (GuhSettings::isRoot()) {
        path = "/etc/guh/";
    } else {
        path = QDir::homePath() + "/.config/" + organisationName;
    }
#endif // SNAPPY
    return path;
}

/*! Returns the default system translation path \tt{/usr/share/guh/translations}. */
QString GuhSettings::translationsPath()
{
#ifdef SNAPPY
    return QString(qgetenv("SNAP_DATA"));
#else
    return QString("/usr/share/guh/translations");
#endif // SNAPPY
}

QString GuhSettings::storagePath()
{
    QString path;
#ifdef SNAPPY
    path = QString(qgetenv("SNAP_DATA"));
#else
    QString organisationName = QCoreApplication::instance()->organizationName();
    if (organisationName == "guh-test") {
        path = "/tmp/" + organisationName + "/";
    } else if (GuhSettings::isRoot()) {
        path = "/var/lib/" + organisationName + "/";
    } else {
        path = QDir::homePath() + "/.local/share/" + organisationName + "/";
    }
#endif
    return path;
}

/*! Return a list of all settings keys.*/
QStringList GuhSettings::allKeys() const
{
    return m_settings->allKeys();
}

/*! Adds \a prefix to the current group and starts writing an array of size size. If size is -1 (the default),
 * it is automatically determined based on the indexes of the entries written. */
void GuhSettings::beginWriteArray(const QString &prefix)
{
    m_settings->beginWriteArray(prefix);
}

/*! Sets the current array index to \a i. */
void GuhSettings::setArrayIndex(int i)
{
    m_settings->setArrayIndex(i);
}

/*! Adds \a prefix to the current group and starts reading from an array. Returns the size of the array.*/
int GuhSettings::beginReadArray(const QString &prefix)
{
    return m_settings->beginReadArray(prefix);
}

/*! End an array. */
void GuhSettings::endArray()
{
    m_settings->endArray();
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
    m_settings->setValue(key, value);
}

/*! Returns the value for setting \a key. If the setting doesn't exist, returns \a defaultValue. */
QVariant GuhSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

