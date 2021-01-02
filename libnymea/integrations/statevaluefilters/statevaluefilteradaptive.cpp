#include "statevaluefilteradaptive.h"

#include <qmath.h>

StateValueFilterAdaptive::StateValueFilterAdaptive()
{

}

void StateValueFilterAdaptive::addValue(const QVariant &value)
{
    qCDebug(dcStateValueFilter()) << "Adding value:" << value.toDouble();
    m_values.prepend(value.toDouble());
    m_inputValues++;
    update();
}

QVariant StateValueFilterAdaptive::filteredValue() const
{
    return m_filteredValue;
}

void StateValueFilterAdaptive::update()
{

//    while (m_values.count() > m_windowSize + 1) {
//        m_values.removeLast();
//    }

//    if (m_values.isEmpty()) {
//        m_filteredValue = 0;
//        return;
//    }

//    if (m_values.count() == 1) {
//        m_filteredValue = m_values.first();
//        m_outputValues++;
//        return;
//    }



////    m_filteredValue = m_values.first();
////    m_outputValues++;


//    double currentValue = m_values.first();
//    if (currentValue == 0) {
//        m_filteredValue = 0;
//        return;
//    }


//    // Calculate average of history, for all values and for all but the last one
//    double sum = 0;
//    for (int i = 1; i < m_values.count(); i++) {
//        sum += m_values.at(i);
//    }
//    double average = sum / (m_values.count() - 1);

//    double absoluteJitter = currentValue - average;
//    double relativeJitter = absoluteJitter / currentValue;


//    // Outside of jitter window... Forward value directly
//    if (qAbs(relativeJitter) > m_averageJitter * 3) {
//        m_filteredValue = m_values.first();
//        m_values.clear();
//        m_values.prepend(m_filteredValue);
//        m_outputValues++;
//        qCDebug(dcStateValueFilter()) << "Updating output";
//    } else {

//    }
//    // Adjust average jitter
//    m_averageJitter = ((m_averageJitter * m_windowSize) + qAbs(relativeJitter)) / (m_windowSize + 1);


//    qCDebug(dcStateValueFilter()) << "input" << currentValue << "output" << m_filteredValue << "average" << average << "jitter:" << absoluteJitter << "relative" << relativeJitter << "avg" << m_averageJitter;
//    qCDebug(dcStateValueFilter()) << "Filter input values:" << m_inputValues << "output values:" << m_outputValues << "compression ratio:" << (1.0 * m_inputValues / m_outputValues);


//    return;





    while (m_values.count() > m_windowSize) {
        m_values.removeLast();
    }

    if (m_values.isEmpty()) {
        m_filteredValue = 0;
        return;
    }

    if (m_values.count() == 1) {
        // Not enough data
        m_filteredValue = m_values.first();
        m_outputValues++;
        return;
    }


    // Calculate average of history, for all values and for all but the last one
    double sum = 0;
    for (int i = 0; i < m_values.count(); i++) {
        sum += m_values.at(i);
    }

    double currentValue = m_values.first();
    if (qFuzzyCompare(currentValue, 0)) {
        m_filteredValue = 0;
        return;
    }

    double filteredValue = sum / m_values.count();
    double previousFilteredValue = (sum - m_values.first()) / (m_values.count() - 1);

    if (qFuzzyCompare(previousFilteredValue, 0)) {
        m_filteredValue = m_values.first();
        m_outputValues++;
        return;
    }

    // Calculate change ratio of the last value compared to the previous one, unflitered and filtered
    double changeRatio = 1 - qAbs(currentValue / previousFilteredValue);
    double changeRatioFiltered = 1 - qAbs(filteredValue / previousFilteredValue);

    // Add deviation of actual value vs filtered value up to have an idea how much we're off
    m_totalDeviation += changeRatioFiltered - changeRatio;


    // If the unfiltered value changes for more than 3 times the standard deviation of the jittering values
    // it's a 99% chance a big change happened that's not jitter (e.g turned on/off)
    // Discard the history and follow the new value right away
    if (qAbs(changeRatio) > m_standardDeviation * 3) {
        m_values.clear();
        m_values.prepend(currentValue);
        m_totalDeviation = 0;
        if (!qFuzzyCompare(m_filteredValue, filteredValue)) {
            m_filteredValue = currentValue;
            qCDebug(dcStateValueFilter()) << "Updating output value:" << m_filteredValue << "(input exceeds max jitter)";
            m_outputValues++;
        }

    // If the filtered value changed for 5 percent or more, follow slowly
    // In order to not get stuck on being off for 5% forever, also move closer
    // to the new value when the deviation exceeds max deviation
    } else if (qAbs(changeRatioFiltered) > m_standardDeviation || qAbs(m_totalDeviation) > m_maxTotalDeviation) {
        m_totalDeviation = 0;
        if (!qFuzzyCompare(m_filteredValue, filteredValue)) {
            qCDebug(dcStateValueFilter()) << "Updating output value:" << filteredValue << "(drift compensation)";
            m_filteredValue = filteredValue;
            m_outputValues++;
        }
    }

    // Poor mans solution to calculate standard deviation. Not as precise, but much faster than looping over history again
    m_standardDeviation = ((m_standardDeviation * m_windowSize) + qAbs(changeRatio)) / (m_windowSize + 1);
    qWarning(dcStateValueFilter()) << "New:" << currentValue << "Old:" << previousFilteredValue << "Filtered:" << filteredValue << "ratio:" << changeRatio << "filteredRatio" << changeRatioFiltered << "deviation" << m_totalDeviation << "averageJitter" << m_averageJitter;

    // correct stats on overflow of counters
    if (m_inputValues < m_outputValues) {
        m_outputValues = 0;
    }

    qCDebug(dcStateValueFilter()) << "Filter input values:" << m_inputValues << "output values:" << m_outputValues << "compression ratio:" << (1.0 * m_inputValues / m_outputValues);
}
