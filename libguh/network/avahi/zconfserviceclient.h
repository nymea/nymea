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

#ifndef ZCONFSERVICECLIENT_H
#define ZCONFSERVICECLIENT_H

#include <QObject>
#include <avahi-client/client.h>

class ZConfServiceClient : public QObject
{
    Q_OBJECT

signals:
    void clientRunning();
    void clientFailure();
    void clientConnecting();
    void clientReset();

private:
    friend class ZConfService;
    friend class ZConfServiceBrowser;
    friend class ZConfServiceBrowserPrivate;

    ZConfServiceClient(QObject *parent = 0);
    ~ZConfServiceClient();

    void run();
    QString errorString() const;

    static void callback(AvahiClient *client, AvahiClientState state, void *userdata);

    const AvahiPoll *const poll;
    AvahiClient     *client;
    int              error;
};

#endif // ZCONFSERVICECLIENT_H
