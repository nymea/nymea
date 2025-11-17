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

#include "statevaluefilteradaptive.h"

#include <qmath.h>

StateValueFilterAdaptive::StateValueFilterAdaptive()
{

}

void StateValueFilterAdaptive::addValue(const QVariant &value)
{
    m_inputValues.prepend(value.toDouble());
    m_inputValueCount++;
    update();
}

QVariant StateValueFilterAdaptive::filteredValue() const
{
    return m_outputValue;
}

void StateValueFilterAdaptive::update()
{
    while (m_inputValues.count() > m_windowSize) {
        m_inputValues.removeLast();
    }

    if (m_inputValues.isEmpty()) {
        m_outputValue = 0;
        return;
    }

    if (m_inputValues.count() == 1) {
        // Not enough data
        m_outputValue = m_inputValues.first();
        m_outputValueCount++;
        return;
    }

    double currentValue = m_inputValues.first();
    if (qFuzzyCompare(currentValue, 0)) {
        // If we went to 0, follow right away.
        m_outputValue = 0;
        return;
    }

    // Calculate average of history, for all values and for all but the last one
    double sum = 0;
    for (int i = 0; i < m_inputValues.count(); i++) {
        sum += m_inputValues.at(i);
    }
    double normalizedValue = sum / m_inputValues.count();
    double previousNormalizedValue = (sum - m_inputValues.first()) / (m_inputValues.count() - 1);

    if (qFuzzyCompare(previousNormalizedValue, 0)) {
        // We can't calculate anything if the history is at 0. Follow right away to the new value.
        m_outputValue = currentValue;
        m_outputValueCount++;
        return;
    }

    // Calculate change ratio of the last value compared to the previous one, unflitered and filtered
    double changeRatioToAverage = 1 - qAbs(currentValue / previousNormalizedValue);
    double changeRatioToCurrentOutput = 1 - qAbs(currentValue / m_outputValue);
    double changeRatioFiltered = 1 - qAbs(normalizedValue / previousNormalizedValue);


    // If the unfiltered value changes for more than 3 times the standard deviation of the jittering values
    // it's a 99% chance a big change happened that's not jitter (e.g turned on/off)
    // Discard the history and follow the new value right away
    if (qAbs(changeRatioToAverage) > m_standardDeviation * 3) {
        m_inputValues.clear();
        m_inputValues.prepend(currentValue);
        m_totalDeviation = 0;
        if (!qFuzzyCompare(m_outputValue, normalizedValue)) {
            m_outputValue = currentValue;
            qCDebug(dcStateValueFilter()) << "Updating output value:" << m_outputValue << "(input exceeds max jitter)";
            m_outputValueCount++;
        }

    // We're considering it jitter
    } else {
        // Add up the deviation from the current actual value to the currently filtered value
        m_totalDeviation += changeRatioToCurrentOutput;

        // If the filtered value changed for more than the standard deviation, follow slowly
        // In order to not get stuck on being off for the standard deviation forever, also move closer
        // to the new value when the summed up deviation exceeds the maximum allowed total deviation
        if (qAbs(changeRatioFiltered) > m_standardDeviation || qAbs(m_totalDeviation) > m_maxTotalDeviation) {
            m_totalDeviation = 0;
            if (!qFuzzyCompare(m_outputValue, normalizedValue)) {
                qCDebug(dcStateValueFilter()) << "Updating output value:" << normalizedValue << "(drift compensation)";
                m_outputValue = normalizedValue;
                m_outputValueCount++;
            }
        }
    }

    // Poor mans solution to calculate standard deviation. Not as precise, but much faster than looping over history again
    m_standardDeviation = ((m_standardDeviation * m_windowSize) + qAbs(changeRatioToAverage)) / (m_windowSize + 1);

    // reset stats on overflow of counters
    if (m_inputValueCount < m_outputValueCount) {
        m_outputValueCount = 0;
    }

    qCDebug(dcStateValueFilter()) << "Filter statistics for" << this;
    qCDebug(dcStateValueFilter()) << "Input:" << currentValue << "AVG:" << previousNormalizedValue << "Filtered:" << normalizedValue;
    qCDebug(dcStateValueFilter()) << "Change ratios: Input/average:" << changeRatioToAverage << "Filtered/average:" << changeRatioFiltered  << "Input/output:" << changeRatioToCurrentOutput;
    qCDebug(dcStateValueFilter()) << "Std deviation:" << m_standardDeviation << "Total deviation:" << m_totalDeviation;
    qCDebug(dcStateValueFilter()) << "Compression ratio:" << (1.0 * m_inputValueCount / m_outputValueCount) << "(" << m_outputValueCount << "/" << m_inputValueCount << ")";
}
