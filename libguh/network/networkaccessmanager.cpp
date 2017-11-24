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
  \class NetworkAccessManager
  \brief Allows to send network requests and receive replies.

  \ingroup hardware
  \inmodule libguh

  The network manager class is a reimplementation of the \l{http://doc-snapshot.qt-project.org/qt5-5.4/qnetworkaccessmanager.html}{QNetworkAccessManager}
  and allows plugins to send network requests and receive replies.

*/

#include "networkaccessmanager.h"
#include "loggingcategories.h"

/*! Construct the hardware resource NetworkAccessManager with the given \a parent. */
NetworkAccessManager::NetworkAccessManager(QNetworkAccessManager *networkManager, QObject *parent) :
    HardwareResource(HardwareResource::TypeNetworkManager, "Network access manager" , parent),
    m_manager(networkManager)
{
    setAvailable(true);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &request)
{
    return m_manager->get(request);
}

QNetworkReply *NetworkAccessManager::deleteResource(const QNetworkRequest &request)
{
    return m_manager->deleteResource(request);
}

QNetworkReply *NetworkAccessManager::head(const QNetworkRequest &request)
{
    return m_manager->head(request);
}

QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &request, QIODevice *data)
{
    return m_manager->post(request, data);
}

QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &request, const QByteArray &data)
{
    return m_manager->post(request, data);
}

QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    return m_manager->post(request, multiPart);
}

QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &request, QIODevice *data)
{
    return m_manager->put(request, data);
}

QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &request, const QByteArray &data)
{
    return m_manager->put(request, data);
}

QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    return m_manager->put(request, multiPart);
}

QNetworkReply *NetworkAccessManager::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
    return m_manager->sendCustomRequest(request, verb, data);
}

bool NetworkAccessManager::enable()
{
    m_manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    setEnabled(true);
    return true;
}

bool NetworkAccessManager::disable()
{
    m_manager->setNetworkAccessible(QNetworkAccessManager::NotAccessible);
    setEnabled(false);
    return true;
}
