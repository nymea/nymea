/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
