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

#ifndef ZWAVEMANAGERREPLY_H
#define ZWAVEMANAGERREPLY_H

#include <QObject>

#include "hardware/zwave/zwavereply.h"

namespace nymeaserver
{

class ZWaveManagerReply : public ZWaveReply
{
    Q_OBJECT
    friend class ZWaveManager;

public:
    explicit ZWaveManagerReply(QObject *parent = nullptr);

private slots:
    void finish(ZWave::ZWaveError status) override;

signals:

};

}
#endif // ZWAVEMANAGERREPLY_H
