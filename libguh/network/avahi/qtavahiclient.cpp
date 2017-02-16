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

#include "qtavahiclient.h"
#include "qt-watch.h"

#include <avahi-common/error.h>

QtAvahiClient::QtAvahiClient(QObject *parent) :
    QObject(parent),
    poll(avahi_qt_poll_get()),
    client(0),
    error(0)
{

}

QtAvahiClient::~QtAvahiClient()
{
    if (client)
        avahi_client_free(client);
}

void QtAvahiClient::start()
{
    if (client)
        return;

    avahi_client_new(poll, (AvahiClientFlags) 0, QtAvahiClient::callback, this, &error);
}

QString QtAvahiClient::errorString() const
{
    return QString(avahi_strerror(error));
}

void QtAvahiClient::callback(AvahiClient *client, AvahiClientState state, void *userdata)
{
    QtAvahiClient *serviceClient = static_cast<QtAvahiClient *>(userdata);
    if (!serviceClient)
        return;

    serviceClient->client = client;

    switch (state) {
    case AVAHI_CLIENT_S_RUNNING:
        emit serviceClient->clientStateChanged(QtAvahiClientStateRunning);
        break;
    case AVAHI_CLIENT_FAILURE:
        emit serviceClient->clientStateChanged(QtAvahiClientStateFailure);
        break;
    case AVAHI_CLIENT_S_COLLISION:
        emit serviceClient->clientStateChanged(QtAvahiClientStateCollision);
        break;
    case AVAHI_CLIENT_S_REGISTERING:
        emit serviceClient->clientStateChanged(QtAvahiClientStateRegistering);
        break;
    case AVAHI_CLIENT_CONNECTING:
        emit serviceClient->clientStateChanged(QtAvahiClientStateConnecting);
        break;
    }
}

