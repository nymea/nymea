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

#include "libnymea.h"
#include "typeutils.h"
#include "hardwareresource.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QUrl>

class LIBNYMEA_EXPORT NetworkAccessManager : public HardwareResource
{
    Q_OBJECT

public:
    NetworkAccessManager(QObject *parent = nullptr);
    virtual ~NetworkAccessManager() = default;

    virtual QNetworkReply *get(const QNetworkRequest &request) = 0;
    virtual QNetworkReply *deleteResource(const QNetworkRequest &request) = 0;
    virtual QNetworkReply *head(const QNetworkRequest &request) = 0;

    virtual QNetworkReply *post(const QNetworkRequest &request, QIODevice *data) = 0;
    virtual QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) = 0;
    virtual QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart) = 0;

    virtual QNetworkReply *put(const QNetworkRequest &request, QIODevice *data) = 0;
    virtual QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data) = 0;
    virtual QNetworkReply *put(const QNetworkRequest &request, QHttpMultiPart *multiPart) = 0;

    virtual QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data = nullptr) = 0;

};

#endif // NETWORKACCESSMANAGER_H
