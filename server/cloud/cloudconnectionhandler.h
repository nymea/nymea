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

#ifndef CLOUDCONNECTIONHANDLER_H
#define CLOUDCONNECTIONHANDLER_H

#include <QObject>

#include "cloud.h"
#include "cloudjsonhandler.h"

namespace guhserver {

class CloudConnectionHandler : public CloudJsonHandler
{
    Q_OBJECT
public:
    explicit CloudConnectionHandler(QObject *parent = 0);

    QString nameSpace() const;

    // API methods
    Q_INVOKABLE void processGetConnections(const QVariantMap &params);
    Q_INVOKABLE void processGetTunnels(const QVariantMap &params);
    Q_INVOKABLE void processSendData(const QVariantMap &params);

    // API notifications
    Q_INVOKABLE void processConnectionAdded(const QVariantMap &params);
    Q_INVOKABLE void processConnectionRemoved(const QVariantMap &params);
    Q_INVOKABLE void processTunnelAdded(const QVariantMap &params);
    Q_INVOKABLE void processTunnelRemoved(const QVariantMap &params);
    Q_INVOKABLE void processDataReceived(const QVariantMap &params);

};

}

#endif // CLOUDCONNECTIONHANDLER_H
