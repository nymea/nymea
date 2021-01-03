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
    QList<double> m_inputValues;

    int m_windowSize = 20;
    double m_standardDeviation = 0.05;
    double m_maxTotalDeviation = 0.4;

    double m_totalDeviation = 0;

    double m_outputValue = 0;

    // Stats for debugging
    quint64 m_inputValueCount = 0;
    quint64 m_outputValueCount = 0;


};

#endif // STATEVALUEFILTERADAPTIVE_H
