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

#include "kodi.h"

Kodi::Kodi(const QHostAddress &hostAddress, const int &port, QObject *parent) :
    QObject(parent)
{
    m_connection = new KodiConnection(hostAddress, port, this);
    connect (m_connection, &KodiConnection::connectionStatusChanged, this, &Kodi::connectionStatusChanged);

    m_jsonHandler = new JsonHandler(m_connection, this);
    connect(m_jsonHandler, &JsonHandler::volumeChanged, this, &Kodi::onVolumeChanged);
    connect(m_jsonHandler, &JsonHandler::actionExecuted, this, &Kodi::actionExecuted);
    connect(m_jsonHandler, &JsonHandler::updateDataReceived, this, &Kodi::onUpdateFinished);
    connect(m_jsonHandler, &JsonHandler::onPlayerPlay, this, &Kodi::onPlayerPlay);
    connect(m_jsonHandler, &JsonHandler::onPlayerPause, this, &Kodi::onPlayerPause);
    connect(m_jsonHandler, &JsonHandler::onPlayerStop, this, &Kodi::onPlayerStop);
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

void Kodi::showNotification(const QString &message, const int &displayTime, const ActionId &actionId)
{
    QVariantMap params;
    params.insert("title", "guh notification");
    params.insert("message", message);
    params.insert("displaytime", displayTime);
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
        m_volume = data.value("muted").toBool();
    }
    emit stateChanged();
}
