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

#include "guhsettings.h"
#include "unistd.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

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
            settingsFile = settingsPrefix + "/test-devices";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created test-devices settings" << m_settings->fileName();
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/devices.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
            qDebug() << "Created device settings" << m_settings->fileName();
        } else {
            settingsFile = settingsPrefix + "/devices";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created device settings" << m_settings->fileName();
        }
        break;
    case SettingsRoleRules:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = settingsPrefix + "/test-rules";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created test-rules settings" << m_settings->fileName();
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/rules.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
            qDebug() << "Created rule settings" << m_settings->fileName();
        } else {
            settingsFile = settingsPrefix + "/rules";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created rule settings" << m_settings->fileName();
        }
        break;
    case SettingsRolePlugins:
        // check if we are running a test
        if (settingsPrefix == "guh-test") {
            settingsFile = settingsPrefix + "/test-plugins";
            m_settings = new QSettings(settingsFile, QSettings::NativeFormat, this);
            qDebug() << "Created test-plugins settings" << m_settings->fileName();
        } else if (rootPrivilege) {
            settingsFile = "/etc/" + settingsPrefix + "/plugins.conf";
            m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
            qDebug() << "Created plugin settings" << m_settings->fileName();
        } else {
            settingsFile = settingsPrefix + "/plugins";
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

GuhSettings::~GuhSettings()
{
    m_settings->sync();
    delete m_settings;
}

GuhSettings::SettingsRole GuhSettings::settingsRole() const
{
    return m_role;
}

bool GuhSettings::isRoot()
{
    if (getuid() != 0)
        return false;

    return true;
}

QString GuhSettings::logPath()
{
    QString logPath;
    QString organisationName = QCoreApplication::instance()->organizationName();

    if (organisationName == "guh-test") {
        logPath = "/tmp/guhd-test.log";
    } else if (GuhSettings::isRoot()) {
        logPath = "/var/log/guhd.log";
    } else {
        logPath = QDir::homePath() + "/.config/" + organisationName + "/guhd.log";
    }

    return logPath;
}

QStringList GuhSettings::allKeys() const
{
    return m_settings->allKeys();
}

void GuhSettings::beginGroup(const QString &prefix)
{
    m_settings->beginGroup(prefix);
}

QStringList GuhSettings::childGroups() const
{
    return m_settings->childGroups();
}

QStringList GuhSettings::childKeys() const
{
    return m_settings->childKeys();
}

void GuhSettings::clear()
{
    m_settings->clear();
}

bool GuhSettings::contains(const QString &key) const
{
    return m_settings->contains(key);
}

void GuhSettings::endGroup()
{
    m_settings->endGroup();
}

QString GuhSettings::group() const
{
    return m_settings->group();
}

QString GuhSettings::fileName() const
{
    return m_settings->fileName();
}

bool GuhSettings::isWritable() const
{
    return m_settings->isWritable();
}

void GuhSettings::remove(const QString &key)
{
    m_settings->remove(key);
}

void GuhSettings::setValue(const QString &key, const QVariant &value)
{
    //Q_ASSERT_X(m_role != GuhSettings::SettingsRoleGlobal, "GuhSettings", "Bad settings usage. The global settings file should be read only.");
    m_settings->setValue(key, value);
}

QVariant GuhSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

