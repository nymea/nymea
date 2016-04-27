/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef KODI_H
#define KODI_H

#include <QObject>
#include <QHostAddress>

#include "kodiconnection.h"
#include "kodijsonhandler.h"

class Kodi : public QObject
{
    Q_OBJECT
public:

    explicit Kodi(const QHostAddress &hostAddress, const int &port = 9090, QObject *parent = 0);

    QHostAddress hostAddress() const;
    int port() const;

    bool connected() const;

    // propertys
    void setMuted(const bool &muted, const ActionId &actionId);
    bool muted() const;

    void setVolume(const int &volume, const ActionId &actionId);
    int volume() const;

    // actions
    void showNotification(const QString &message, const int &displayTime, const QString &notificationType, const ActionId &actionId);
    void pressButton(const QString &button, const ActionId &actionId);
    void systemCommand(const QString &command, const ActionId &actionId);
    void videoLibrary(const QString &command, const ActionId &actionId);
    void audioLibrary(const QString &command, const ActionId &actionId);

    void update();
    void checkVersion();

    void connectKodi();
    void disconnectKodi();

private:
    KodiConnection *m_connection;
    KodiJsonHandler *m_jsonHandler;
    bool m_muted;
    int m_volume;

signals:
    void connectionStatusChanged();
    void stateChanged();
    void actionExecuted(const ActionId &actionId, const bool &success);
    void updateDataReceived(const QVariantMap &data);
    void versionDataReceived(const QVariantMap &data);

    void onPlayerPlay();
    void onPlayerPause();
    void onPlayerStop();

private slots:
    void onVolumeChanged(const int &volume, const bool &muted);
    void onUpdateFinished(const QVariantMap &data);


};

#endif // KODI_H
