/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef CLOUDINTERFACE_H
#define CLOUDINTERFACE_H

#include <QHash>
#include <QUuid>
#include <QObject>
#include <QVariantMap>

#include "cloud.h"
#include "cloudjsonreply.h"
#include "cloudconnectionhandler.h"
#include "cloudauthenticationhandler.h"

namespace guhserver {

class CloudInterface : public QObject
{
    Q_OBJECT
public:
    explicit CloudInterface(QObject *parent = 0);

    Q_INVOKABLE void authenticateConnection(const QString &token);
    Q_INVOKABLE void getTunnels();
    Q_INVOKABLE void sendApiData(const QUuid &tunnelId, const QVariantMap &data);

private:
    int m_id;
    QUuid m_guhUuid;

    QHash<QString, CloudJsonHandler *> m_handlers;
    QHash<int, CloudJsonReply *> m_replies;

    CloudJsonReply *createReply(QString nameSpace, QString method, QVariantMap params = QVariantMap());

    CloudAuthenticationHandler *m_authenticationHandler;
    CloudConnectionHandler *m_connectionHandler;

signals:
    void responseReceived(const int &commandId, const QVariantMap &response);

public slots:
    void dataReceived(const QVariantMap &data);

};

}

#endif // CLOUDINTERFACE_H
