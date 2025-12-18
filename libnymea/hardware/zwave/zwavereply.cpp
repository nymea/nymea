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

#include "zwavereply.h"

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcZWave)

ZWaveReply::ZWaveReply(QObject *parent)
    : QObject{parent}
{
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        qCWarning(dcZWave) << "ZWaveReply timed out...";
        finish(ZWave::ZWaveErrorTimeout);
    });
    connect(this, &ZWaveReply::finished, this, &ZWaveReply::deleteLater);
}

void ZWaveReply::start(int timeout)
{
    m_timer.start(timeout);
}

void ZWaveReply::finish(ZWave::ZWaveError status)
{
    if (m_finished) {
        qCWarning(dcZWave) << "Reply already finished. Not finishing a second time.";
        return;
    }
    m_finished = true;
    m_timer.stop();
    // Delaying for one event loop pass to give the user chance to connect even if the reply is finished before returning
    QTimer::singleShot(0, this, [this, status]() { emit finished(status); });
}
