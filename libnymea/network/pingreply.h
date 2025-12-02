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

#ifndef PINGREPLY_H
#define PINGREPLY_H

#include <QTimer>
#include <QObject>
#include <QHostAddress>

#include <sys/time.h>

#include "libnymea.h"

#include <QHostAddress>
#include <QNetworkInterface>

class LIBNYMEA_EXPORT PingReply : public QObject
{
    Q_OBJECT

    friend class Ping;

public:
    enum Error {
        ErrorNoError,
        ErrorAborted,
        ErrorInvalidResponse,
        ErrorNetworkDown,
        ErrorNetworkUnreachable,
        ErrorPermissionDenied,
        ErrorSocketError,
        ErrorTimeout,
        ErrorHostUnreachable,
        ErrorInvalidHostAddress,
        ErrorHostNameLookupFailed,
        ErrorHostNameNotFound
    };
    Q_ENUM(Error)

    explicit PingReply(QObject *parent = nullptr);

    QHostAddress targetHostAddress() const;
    quint16 sequenceNumber() const;
    quint16 requestId() const;
    QString hostName() const;
    QNetworkInterface networkInterface() const;

    uint retries() const;
    uint retryCount() const;

    double duration() const;

    Error error() const;

    bool doHostLookup() const;

public slots:
    void abort();

signals:
    void finished();
    void timeout();
    void retry(Error error, uint retryCount);
    void aborted();

private:
    QTimer *m_timer = nullptr;
    QHostAddress m_targetHostAddress;
    quint16 m_sequenceNumber = 1;
    quint16 m_requestId = 0;
    QString m_hostName;
    QNetworkInterface m_networkInterface;

    bool m_doHostLookup = false;

    uint m_retries = 0;
    uint m_retryCount = 0;
    uint m_timeout = 3;
    double m_duration = 0;
    Error m_error = ErrorNoError;

    struct timeval m_startTime;

};

#endif // PINGREPLY_H
