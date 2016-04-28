/*
 *  This file is part of qtzeroconf. (c) 2012 Johannes Hilden
 *  https://github.com/johanneshilden/qtzeroconf
 *
 *  Modified: (C) 2016 Simon Stürz <stuerz.simon@gmail.com>
 *
 *  qtzeroconf is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  qtzeroconf is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with qtzeroconf; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <QDebug>
#include <avahi-common/error.h>
#include "zconfserviceclient.h"
#include "qt-watch.h"

void ZConfServiceClient::run()
{
    if (client)
        return;
    avahi_client_new(poll, (AvahiClientFlags) 0, ZConfServiceClient::callback, this, &error);
}

QString ZConfServiceClient::errorString() const
{
    return avahi_strerror(error);
}

ZConfServiceClient::ZConfServiceClient(QObject *parent)
    : QObject(parent),
      poll(avahi_qt_poll_get()),
      client(0),
      error(0)
{
}

ZConfServiceClient::~ZConfServiceClient()
{
    if (client)
        // This will automatically free all associated browser,
        // resolve and entry group objects.
        avahi_client_free(client);
}

void ZConfServiceClient::callback(AvahiClient *client, AvahiClientState state, void *userdata)
{
    ZConfServiceClient *service = static_cast<ZConfServiceClient *>(userdata);
    if (service) {
        service->client = client;
        switch (state)
        {
        case AVAHI_CLIENT_S_RUNNING:
            qDebug() << "AVAHI_CLIENT_S_RUNNING";
            // The server has started up successfully and registered its host
            // name on the network.
            emit service->clientRunning();
            break;
        case AVAHI_CLIENT_FAILURE:
            qDebug() << "AVAHI_CLIENT_FAILURE";
            emit service->clientFailure();
            break;
        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:
            qDebug() << (AVAHI_CLIENT_S_COLLISION == state
                        ? "AVAHI_CLIENT_S_COLLISION"
                        : "AVAHI_CLIENT_S_REGISTERING");
            emit service->clientReset();
            break;
        case AVAHI_CLIENT_CONNECTING:
            qDebug() << "AVAHI_CLIENT_CONNECTING";
            emit service->clientConnecting();
        } // end switch
    }
}
