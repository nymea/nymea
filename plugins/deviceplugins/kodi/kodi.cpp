/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "kodi.h"

Kodi::Kodi(const QHostAddress &hostAddress, const int &port, QObject *parent) :
    QObject(parent),
    m_muted(false),
    m_volume(-1)
{
    m_connection = new KodiConnection(hostAddress, port, this);
    connect (m_connection, &KodiConnection::connectionStatusChanged, this, &Kodi::connectionStatusChanged);

    m_jsonHandler = new KodiJsonHandler(m_connection, this);
    connect(m_jsonHandler, &KodiJsonHandler::volumeChanged, this, &Kodi::onVolumeChanged);
    connect(m_jsonHandler, &KodiJsonHandler::actionExecuted, this, &Kodi::actionExecuted);
    connect(m_jsonHandler, &KodiJsonHandler::versionDataReceived, this, &Kodi::versionDataReceived);
    connect(m_jsonHandler, &KodiJsonHandler::updateDataReceived, this, &Kodi::updateDataReceived);
    connect(m_jsonHandler, &KodiJsonHandler::updateDataReceived, this, &Kodi::onUpdateFinished);
    connect(m_jsonHandler, &KodiJsonHandler::onPlayerPlay, this, &Kodi::onPlayerPlay);
    connect(m_jsonHandler, &KodiJsonHandler::onPlayerPause, this, &Kodi::onPlayerPause);
    connect(m_jsonHandler, &KodiJsonHandler::onPlayerStop, this, &Kodi::onPlayerStop);
}

QHostAddress Kodi::hostAddress() const
{
    return m_connection->hostAddress();
}

int Kodi::port() const
{
    return m_connection->port();
}

bool Kodi::connected() const
{
    return m_connection->connected();
}

void Kodi::setMuted(const bool &muted, const ActionId &actionId)
{
    QVariantMap params;
    params.insert("mute", muted);

    m_jsonHandler->sendData("Application.SetMute", params, actionId);
}

bool Kodi::muted() const
{
    return m_muted;
}

void Kodi::setVolume(const int &volume, const ActionId &actionId)
{
    QVariantMap params;
    params.insert("volume", volume);

    m_jsonHandler->sendData("Application.SetVolume", params, actionId);
}

int Kodi::volume() const
{
    return m_volume;
}

void Kodi::showNotification(const QString &message, const int &displayTime, const QString &notificationType, const ActionId &actionId)
{
    QVariantMap params;
    params.insert("title", "guh notification");
    params.insert("message", message);
    params.insert("displaytime", displayTime);
    params.insert("image", notificationType);

    m_jsonHandler->sendData("GUI.ShowNotification", params, actionId);
}

void Kodi::pressButton(const QString &button, const ActionId &actionId)
{
    QVariantMap params;
    params.insert("action", button);
    m_jsonHandler->sendData("Input.ExecuteAction", params, actionId);
}

void Kodi::systemCommand(const QString &command, const ActionId &actionId)
{
    QString method;
    if (command == "hibernate") {
        method = "Hibernate";
    } else if (command == "reboot") {
        method = "Reboot";
    } else if (command == "shutdown") {
        method = "Shutdown";
    } else if (command == "suspend") {
        method = "Suspend";
    } else {
        // already checkt with allowed values
    }

    m_jsonHandler->sendData("System." + method, QVariantMap(), actionId);
}

void Kodi::videoLibrary(const QString &command, const ActionId &actionId)
{
    QString method;
    if (command == "scan") {
        method = "Scan";
    } else if (command == "clean") {
        method = "Clean";
    } else {
        // already checkt with allowed values
    }

    m_jsonHandler->sendData("VideoLibrary." + method, QVariantMap(), actionId);
}

void Kodi::audioLibrary(const QString &command, const ActionId &actionId)
{
    QString method;
    if (command == "scan") {
        method = "Scan";
    } else if (command == "clean") {
        method = "Clean";
    } else {
        // already checkt with allowed values
    }

    m_jsonHandler->sendData("AudioLibrary." + method, QVariantMap(), actionId);
}

void Kodi::update()
{
    QVariantMap params;
    QVariantList properties;
    properties.append("volume");
    properties.append("muted");
    properties.append("name");
    properties.append("version");
    params.insert("properties", properties);

    m_jsonHandler->sendData("Application.GetProperties", params, ActionId());
}

void Kodi::checkVersion()
{
    m_jsonHandler->sendData("JSONRPC.Version", QVariantMap(), ActionId());
}

void Kodi::connectKodi()
{
    m_connection->connectKodi();
}

void Kodi::disconnectKodi()
{
    m_connection->disconnectKodi();
}

void Kodi::onVolumeChanged(const int &volume, const bool &muted)
{
    if (m_volume != volume || m_muted != muted) {
        m_volume = volume;
        m_muted = muted;
        emit stateChanged();
    }
}

void Kodi::onUpdateFinished(const QVariantMap &data)
{
    if (data.contains("volume")) {
        m_volume = data.value("volume").toInt();
    }
    if (data.contains("muted")) {
        m_muted = data.value("muted").toBool();
    }
    emit stateChanged();
}
