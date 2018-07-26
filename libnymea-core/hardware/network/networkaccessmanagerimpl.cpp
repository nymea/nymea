/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

/*!
  \class nymeaserver::NetworkAccessManagerImpl
  \brief Allows to send network requests and receive replies.

  \ingroup hardware
  \inmodule libnymea

  The network manager class is a reimplementation of the \l{http://doc-snapshot.qt-project.org/qt5-5.4/qnetworkaccessmanager.html}{QNetworkAccessManager}
  and allows plugins to send network requests and receive replies.

*/

#include "networkaccessmanagerimpl.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Construct the hardware resource NetworkAccessManagerImpl with the given \a parent. */
NetworkAccessManagerImpl::NetworkAccessManagerImpl(QNetworkAccessManager *networkManager, QObject *parent) :
    NetworkAccessManager(parent),
    m_manager(networkManager)
{
    m_available = true;

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

QNetworkReply *NetworkAccessManagerImpl::get(const QNetworkRequest &request)
{
    QNetworkReply *reply = m_manager->get(request);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::deleteResource(const QNetworkRequest &request)
{
    QNetworkReply *reply = m_manager->deleteResource(request);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::head(const QNetworkRequest &request)
{
    QNetworkReply *reply = m_manager->head(request);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::post(const QNetworkRequest &request, QIODevice *data)
{
    QNetworkReply *reply = m_manager->post(request, data);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::post(const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply *reply = m_manager->post(request, data);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::post(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    QNetworkReply *reply = m_manager->post(request, multiPart);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::put(const QNetworkRequest &request, QIODevice *data)
{
    QNetworkReply  *reply = m_manager->put(request, data);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::put(const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply *reply = m_manager->put(request, data);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::put(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    QNetworkReply *reply = m_manager->put(request, multiPart);
    hookupTimeoutTimer(reply);
    return reply;
}

QNetworkReply *NetworkAccessManagerImpl::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
    QNetworkReply* reply = m_manager->sendCustomRequest(request, verb, data);
    hookupTimeoutTimer(reply);
    return reply;
}

void NetworkAccessManagerImpl::setEnabled(bool enabled)
{
    if (!m_available) {
        qCWarning(dcNetworkManager()) << "NetworkManager not available, cannot enable";
        return;
    }

    if (m_enabled == enabled) {
        qCDebug(dcNetworkManager()) << "Network Manager already" << (enabled ? "enabled" : "disabled") << "... Not changing state.";
        return;
    }

    if (enabled) {
        m_manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
        qCDebug(dcNetworkManager()) << "Network Manager enabled";
    } else {
        m_manager->setNetworkAccessible(QNetworkAccessManager::NotAccessible);
        qCDebug(dcNetworkManager()) << "Network Manager disabled";
    }
    m_enabled = enabled;
}

void NetworkAccessManagerImpl::hookupTimeoutTimer(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::finished, this, &NetworkAccessManagerImpl::networkReplyFinished);
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &NetworkAccessManagerImpl::networkTimeout);
    timer->setSingleShot(true);
    timer->start(30000);
    m_timeoutTimers.insert(reply, timer);
}

void NetworkAccessManagerImpl::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    QTimer *timer = m_timeoutTimers.take(reply);
    timer->stop();
    timer->deleteLater();
}

void NetworkAccessManagerImpl::networkTimeout()
{
    QTimer *timer = static_cast<QTimer*>(sender());
    QNetworkReply *reply = m_timeoutTimers.key(timer);
    qCDebug(dcNetworkManager()) << "Network request timeout for:" << reply->request().url();
    reply->abort();
}

bool NetworkAccessManagerImpl::available() const
{
    return m_available;
}

bool NetworkAccessManagerImpl::enabled() const
{
    return m_enabled;
}

}
