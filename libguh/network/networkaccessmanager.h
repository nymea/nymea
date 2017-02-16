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

#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include "libguh.h"
#include "typeutils.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QUrl>

class LIBGUH_EXPORT NetworkAccessManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkAccessManager(QObject *parent = 0);

    QNetworkReply *get(const PluginId &pluginId, const QNetworkRequest &request);
    QNetworkReply *post(const PluginId &pluginId, const QNetworkRequest &request, const QByteArray &data);
    QNetworkReply *put(const PluginId &pluginId, const QNetworkRequest &request, const QByteArray &data);

private:
    QNetworkAccessManager *m_manager;
    QHash<QNetworkReply *, PluginId> m_replies;

signals:
    void replyReady(const PluginId &pluginId, QNetworkReply *reply);

private slots:
    void replyFinished(QNetworkReply *reply);

};

#endif // NETWORKACCESSMANAGER_H
