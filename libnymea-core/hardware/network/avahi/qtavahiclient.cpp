/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

#include "qt-watch.h"
#include "qtavahiclient.h"
#include "loggingcategories.h"

#include "qtavahiservicebrowserimplementation.h"
#include "qtavahiservicebrowserimplementation_p.h"

#include <avahi-common/error.h>

namespace nymeaserver {

QtAvahiClient::QtAvahiClient(QObject *parent) :
    QObject(parent),
    m_poll(avahi_qt_poll_get()),
    m_client(nullptr),
    error(0),
    m_state(QtAvahiClientStateNone)
{
    connect(this, &QtAvahiClient::clientStateChangedInternal, this, &QtAvahiClient::onClientStateChanged);
}

QtAvahiClient::~QtAvahiClient()
{
    if (m_client)
        avahi_client_free(m_client);

}

QtAvahiClient::QtAvahiClientState QtAvahiClient::state() const
{
    return m_state;
}

void QtAvahiClient::start()
{
    if (m_client)
        return;

    m_client = avahi_client_new(m_poll, (AvahiClientFlags) 0, QtAvahiClient::callback, this, &error);
}

void QtAvahiClient::stop()
{
    if (m_client)
        avahi_client_free(m_client);

    m_client = nullptr;
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

    serviceClient->m_client = client;

    switch (state) {
    case AVAHI_CLIENT_S_RUNNING:
        emit serviceClient->clientStateChangedInternal(QtAvahiClientStateRunning);
        break;
    case AVAHI_CLIENT_FAILURE:
        emit serviceClient->clientStateChangedInternal(QtAvahiClientStateFailure);
        break;
    case AVAHI_CLIENT_S_COLLISION:
        emit serviceClient->clientStateChangedInternal(QtAvahiClientStateCollision);
        break;
    case AVAHI_CLIENT_S_REGISTERING:
        emit serviceClient->clientStateChangedInternal(QtAvahiClientStateRegistering);
        break;
    case AVAHI_CLIENT_CONNECTING:
        emit serviceClient->clientStateChangedInternal(QtAvahiClientStateConnecting);
        break;
    }
}

void QtAvahiClient::onClientStateChanged(const QtAvahiClient::QtAvahiClientState &state)
{
    if (m_state == state)
        return;

    m_state = state;

//    switch (m_state) {
//    case QtAvahiClientStateNone:
//        break;
//    case QtAvahiClientStateRunning:
//        qCDebug(dcAvahi()) << "Client running.";
//        break;
//    case QtAvahiClientStateFailure:
//        qCWarning(dcAvahi()) << "Client failure:" << errorString();
//        break;
//    case QtAvahiClientStateCollision:
//        qCWarning(dcAvahi()) << "Client collision:" << errorString();
//        break;
//    case QtAvahiClientStateRegistering:
//        qCDebug(dcAvahi()) << "Client registering...";
//        break;
//    case QtAvahiClientStateConnecting:
//        qCDebug(dcAvahi()) << "Client connecting...";
//        break;
//    default:
//        break;
//    }

    emit clientStateChanged(m_state);
}

}
