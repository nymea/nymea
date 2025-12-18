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

#ifndef ZWAVEREPLY_H
#define ZWAVEREPLY_H

#include "zwave.h"
#include <QObject>
#include <QTimer>

class ZWaveReply : public QObject
{
    Q_OBJECT
    friend class ZWaveBackend;

public:
    explicit ZWaveReply(QObject *parent = nullptr);
    virtual ~ZWaveReply() = default;

signals:
    void finished(ZWave::ZWaveError status);

protected slots:
    virtual void start(int timeout = 15);
    virtual void finish(ZWave::ZWaveError status);

private:
    QTimer m_timer;
    bool m_finished = false;
};

#endif // ZWAVEREPLY_H
