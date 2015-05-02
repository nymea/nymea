/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "typeutils.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QUrl>



class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = 0);

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

#endif // NETWORKMANAGER_H
