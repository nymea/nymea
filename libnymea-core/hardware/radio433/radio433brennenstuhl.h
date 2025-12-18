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

#ifndef RADIO433BRENNENSTUHL_H
#define RADIO433BRENNENSTUHL_H

#include <QObject>

#include "hardware/radio433/radio433.h"
#include "hardwareresource.h"
#include "libnymea.h"
#include "radio433brennenstuhlgateway.h"

namespace nymeaserver {

class Radio433Brennenstuhl : public Radio433
{
    Q_OBJECT

public:
    explicit Radio433Brennenstuhl(QObject *parent = nullptr);

    bool available() const override;
    bool enabled() const override;

public slots:
    bool sendData(int delay, QList<int> rawData, int repetitions) override;

private slots:
    void brennenstuhlAvailableChanged(bool available);

protected:
    void setEnabled(bool enabled) override;

private:
    Radio433BrennenstuhlGateway *m_brennenstuhlTransmitter;
    bool m_available = false;
    bool m_enabled = false;
};

} // namespace nymeaserver

#endif // RADIO433BRENENSTUHL_H
