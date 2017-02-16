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

#ifndef KODIJSONHANDLER_H
#define KODIJSONHANDLER_H

#include <QObject>
#include <QVariant>
#include <QHash>

#include "kodiconnection.h"
#include "kodireply.h"
#include "typeutils.h"

class KodiJsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit KodiJsonHandler(KodiConnection *connection = 0, QObject *parent = 0);

    void sendData(const QString &method, const QVariantMap &params, const ActionId &actionId);

private:
    KodiConnection *m_connection;
    int m_id;
    QHash<int, KodiReply> m_replys;

    void processNotification(const QString &method, const QVariantMap &params);
    void processActionResponse(const KodiReply &reply, const QVariantMap &response);
    void processRequestResponse(const KodiReply &reply, const QVariantMap &response);

signals:
    void volumeChanged(const int &volume, const bool &muted);
    void actionExecuted(const ActionId &actionId, const bool &success);
    void updateDataReceived(const QVariantMap &data);
    void versionDataReceived(const QVariantMap &data);

    void onPlayerPlay();
    void onPlayerPause();
    void onPlayerStop();

private slots:
    void processResponse(const QByteArray &data);

};

#endif // KODIJSONHANDLER_H
