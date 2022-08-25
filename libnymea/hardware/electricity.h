/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

    static Phases convertPhasesFromString(const QString &phasesString) {
        Phases phases = PhaseUnknown;

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
