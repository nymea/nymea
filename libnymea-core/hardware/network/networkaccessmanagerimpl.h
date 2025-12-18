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

#ifndef NETWORKACCESSMANAGERIMPL_H
#define NETWORKACCESSMANAGERIMPL_H

#include "libnymea.h"
#include "network/networkaccessmanager.h"
#include "typeutils.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QUrl>

namespace nymeaserver {

class NetworkAccessManagerImpl : public NetworkAccessManager
{
    Q_OBJECT

public:
    NetworkAccessManagerImpl(QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    QNetworkReply *get(const QNetworkRequest &request) override;
    QNetworkReply *deleteResource(const QNetworkRequest &request) override;
    QNetworkReply *head(const QNetworkRequest &request) override;

    QNetworkReply *post(const QNetworkRequest &request, QIODevice *data) override;
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart) override;

    QNetworkReply *put(const QNetworkRequest &request, QIODevice *data) override;
    QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *put(const QNetworkRequest &request, QHttpMultiPart *multiPart) override;

    QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data = nullptr) override;

    bool available() const override;
    bool enabled() const override;

protected:
    void setEnabled(bool enabled) override;

private:
    bool m_available = false;
    bool m_enabled = false;

    QNetworkAccessManager *m_manager;
    QHash<QNetworkReply *, QTimer *> m_timeoutTimers;

    void hookupTimeoutTimer(QNetworkReply *reply);

private slots:
    void networkReplyFinished();
    void networkTimeout();
};

} // namespace nymeaserver

#endif // NETWORKACCESSMANAGER_H
