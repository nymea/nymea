/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef QTAVAHICLIENT_H
#define QTAVAHICLIENT_H

#include <QObject>
#include <avahi-client/client.h>

#include "libguh.h"

class LIBGUH_EXPORT QtAvahiClient : public QObject
{
    Q_OBJECT
    Q_ENUMS(QtAvahiClientState)

public:
    enum QtAvahiClientState {
        QtAvahiClientStateRunning,
        QtAvahiClientStateFailure,
        QtAvahiClientStateCollision,
        QtAvahiClientStateRegistering,
        QtAvahiClientStateConnecting
    };

    explicit QtAvahiClient(QObject *parent = 0);
    ~QtAvahiClient();

private:
    friend class QtAvahiService;
    friend class QtAvahiServiceBrowser;
    friend class QtAvahiServiceBrowserPrivate;

    const AvahiPoll *poll;
    AvahiClient *client;
    int error;

    void start();
    QString errorString() const;

    static void callback(AvahiClient *client, AvahiClientState state, void *userdata);

signals:
    void clientStateChanged(const QtAvahiClientState &state);

};

#endif // QTAVAHICLIENT_H
