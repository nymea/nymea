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

/*!
  \class NetworkAccessManagerImpl
  \brief Allows to send network requests and receive replies.

  \ingroup hardware
  \inmodule libguh

  The network manager class is a reimplementation of the \l{http://doc-snapshot.qt-project.org/qt5-5.4/qnetworkaccessmanager.html}{QNetworkAccessManager}
  and allows plugins to send network requests and receive replies.

*/

#include "networkaccessmanagerimpl.h"
#include "loggingcategories.h"

namespace guhserver {

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
    return m_manager->get(request);
}

QNetworkReply *NetworkAccessManagerImpl::deleteResource(const QNetworkRequest &request)
{
    return m_manager->deleteResource(request);
}

QNetworkReply *NetworkAccessManagerImpl::head(const QNetworkRequest &request)
{
    return m_manager->head(request);
}

QNetworkReply *NetworkAccessManagerImpl::post(const QNetworkRequest &request, QIODevice *data)
{
    return m_manager->post(request, data);
}

QNetworkReply *NetworkAccessManagerImpl::post(const QNetworkRequest &request, const QByteArray &data)
{
    return m_manager->post(request, data);
}

QNetworkReply *NetworkAccessManagerImpl::post(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    return m_manager->post(request, multiPart);
}

QNetworkReply *NetworkAccessManagerImpl::put(const QNetworkRequest &request, QIODevice *data)
{
    return m_manager->put(request, data);
}

QNetworkReply *NetworkAccessManagerImpl::put(const QNetworkRequest &request, const QByteArray &data)
{
    return m_manager->put(request, data);
}

QNetworkReply *NetworkAccessManagerImpl::put(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    return m_manager->put(request, multiPart);
}

QNetworkReply *NetworkAccessManagerImpl::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
    return m_manager->sendCustomRequest(request, verb, data);
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

bool NetworkAccessManagerImpl::available() const
{
    return m_available;
}

bool NetworkAccessManagerImpl::enabled() const
{
    return m_enabled;
}

}
