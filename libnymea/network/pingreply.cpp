/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
