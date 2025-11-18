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

#ifndef ELECTRICITY_H
#define ELECTRICITY_H

#include "libnymea.h"

#include <QString>
#include <qobjectdefs.h>

class LIBNYMEA_EXPORT Electricity
{
    Q_GADGET
public:
    enum Phase {
        PhaseNone = 0x00,
        PhaseA = 0x01,
        PhaseB = 0x02,
        PhaseC = 0x04,
        PhaseAll = 0x07,
        PhaseUnknown = 0xff
    };
    Q_DECLARE_FLAGS(Phases, Phase)
    Q_FLAG(Phases)

    static QString convertPhasesToString(const Phases &phases) {

        QString phasesString;

        if (phases.testFlag(PhaseA))
            phasesString.append("A");

        if (phases.testFlag(PhaseB))
            phasesString.append("B");

        if (phases.testFlag(PhaseC))
            phasesString.append("C");

        return phasesString;
    };

    static Phases convertPhasesFromString(const QString &phasesString) {
        Phases phases = PhaseNone;

        if (phasesString.contains("A"))
            phases |= PhaseA;

        if (phasesString.contains("B"))
            phases |= PhaseB;

        if (phasesString.contains("C"))
            phases |= PhaseC;

        return phases;
    };

    static uint getPhaseCount(Phases phases) {
        uint count = 0;

        if (phases.testFlag(PhaseA))
            count++;

        if (phases.testFlag(PhaseB))
            count++;

        if (phases.testFlag(PhaseC))
            count++;

        return count;
    }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Electricity::Phases)

#endif // ELECTRICITY_H
