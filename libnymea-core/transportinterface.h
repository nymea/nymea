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

#ifndef TRANSPORTINTERFACE_H
#define TRANSPORTINTERFACE_H

#include <QList>
#include <QString>
#include <QUuid>
#include <QVariant>

#include "nymeaconfiguration.h"

namespace nymeaserver {

class TransportInterface : public QObject
{
    Q_OBJECT
public:
    explicit TransportInterface(const ServerConfiguration &config, QObject *parent = nullptr);
    virtual ~TransportInterface() = 0;

    virtual void sendData(const QUuid &clientId, const QByteArray &data) = 0;
    virtual void sendData(const QList<QUuid> &clients, const QByteArray &data) = 0;

    virtual void terminateClientConnection(const QUuid &clientId) = 0;

    void setConfiguration(const ServerConfiguration &config);
    ServerConfiguration configuration() const;

protected:
    QString m_serverName;

signals:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QByteArray &data);

public slots:
    virtual void setServerName(const QString &serverName);
    virtual bool startServer() = 0;
    virtual bool stopServer() = 0;

private:
    ServerConfiguration m_config;
};

} // namespace nymeaserver

#endif // TRANSPORTINTERFACE_H
