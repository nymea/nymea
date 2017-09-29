/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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
        QtAvahiClientStateNone,
        QtAvahiClientStateRunning,
        QtAvahiClientStateFailure,
        QtAvahiClientStateCollision,
        QtAvahiClientStateRegistering,
        QtAvahiClientStateConnecting
    };

    explicit QtAvahiClient(QObject *parent = 0);
    ~QtAvahiClient();

    QtAvahiClientState state() const;

private:
    friend class QtAvahiService;
    friend class QtAvahiServiceBrowser;
    friend class QtAvahiServiceBrowserPrivate;

    const AvahiPoll *poll;
    AvahiClient *client;
    int error;
    QtAvahiClientState m_state;

    void start();
    QString errorString() const;

    static void callback(AvahiClient *client, AvahiClientState state, void *userdata);

private slots:
    void onClientStateChanged(const QtAvahiClientState &state);

signals:
    void clientStateChanged(const QtAvahiClientState &state);
    void clientStateChangedInternal(const QtAvahiClientState &state);

};

#endif // QTAVAHICLIENT_H
