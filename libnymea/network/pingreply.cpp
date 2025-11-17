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

#include "pingreply.h"

PingReply::PingReply(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &PingReply::timeout);
    connect(this, &PingReply::finished, m_timer, &QTimer::stop);
}

QHostAddress PingReply::targetHostAddress() const
{
    return m_targetHostAddress;
}

quint16 PingReply::sequenceNumber() const
{
    return m_sequenceNumber;
}

quint16 PingReply::requestId() const
{
    return m_requestId;
}

QString PingReply::hostName() const
{
    return m_hostName;
}

QNetworkInterface PingReply::networkInterface() const
{
    return m_networkInterface;
}

uint PingReply::retries() const
{
    return m_retries;
}

uint PingReply::retryCount() const
{
    return m_retryCount;
}

double PingReply::duration() const
{
    return m_duration;
}

PingReply::Error PingReply::error() const
{
    return m_error;
}

bool PingReply::doHostLookup() const
{
    return m_doHostLookup;
}

void PingReply::abort()
{
    m_timer->stop();
    m_error = ErrorAborted;
    emit aborted();
}
