#ifndef STATEVALUEFILTERADAPTIVE_H
#define STATEVALUEFILTERADAPTIVE_H

#include "statevaluefilter.h"

class StateValueFilterAdaptive : public StateValueFilter
{
public:
    StateValueFilterAdaptive();

    void addValue(const QVariant &value) override;
    QVariant filteredValue() const override;

private:
    void update();

private:
    QList<double> m_values;

    int m_windowSize = 20;
    double m_standardDeviation = 0.05;
    double m_maxTotalDeviation = 1;

    double m_filteredValue = 0;
    double m_totalDeviation = 0;

    // Stats for debugging
    quint64 m_inputValues = 0;
    quint64 m_outputValues = 0;


};

#endif // STATEVALUEFILTERADAPTIVE_H
