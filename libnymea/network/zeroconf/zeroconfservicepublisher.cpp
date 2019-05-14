/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
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

#include "zeroconfservicepublisher.h"

ZeroConfServicePublisher::ZeroConfServicePublisher(QObject *parent) : QObject(parent)
{

}

/*!
 * \brief ZeroConfServicePublisher::registerService
 * \param id An ID used to track this service instance. Pick a unique id and use the same for updating or unregistering the service
 * \param name The name as it should appear on the network. Note that it might be changed when collisions appear
 * \param hostAddress The hostAddress the server is running
 * \param port The port of the server
 * \param serviceType The service type as it should appear on the network, for instance "_http.tcp"
 * \param txtRecords A Map of txt records that should be published with this service
 * \return
 */

bool ZeroConfServicePublisher::registerService(const QString &id, const QString &name, const QHostAddress &hostAddress, const quint16 &port, const QString &serviceType, const QHash<QString, QString> &txtRecords)
{
    Q_UNUSED(id)
    Q_UNUSED(name)
    Q_UNUSED(hostAddress)
    Q_UNUSED(port)
    Q_UNUSED(serviceType)
    Q_UNUSED(txtRecords)
    return false;
}

/*!
 * \brief ZeroConfServicePublisher::unregisterService
 * \param id The id previously used to register the server
 */
void ZeroConfServicePublisher::unregisterService(const QString &id)
{
    Q_UNUSED(id)
}
