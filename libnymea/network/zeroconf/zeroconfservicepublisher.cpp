// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zeroconfservicepublisher.h"

ZeroConfServicePublisher::ZeroConfServicePublisher(QObject *parent) : QObject(parent)
{

}

/*!
 * \brief ZeroConfServicePublisher::registerService
 * \param name The name as it should appear on the network. This name must be unique across all registered services and is used to unregister/update. Note that the actual name as it appears on the network might be changed when collisions with other hosts happen.
 * \param hostAddress The hostAddress the server is running
 * \param port The port of the server
 * \param serviceType The service type as it should appear on the network, for instance "_http.tcp"
 * \param txtRecords A Map of txt records that should be published with this service
 * \return
 */

bool ZeroConfServicePublisher::registerService(const QString &name, const QHostAddress &hostAddress, const quint16 &port, const QString &serviceType, const QHash<QString, QString> &txtRecords)
{
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
