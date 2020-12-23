#ifndef STATEVALUEFILTERADAPTIVE_H
#define STATEVALUEFILTERADAPTIVE_H

#include "statevaluefilter.h"

class StateValueFilterAdaptive : public StateValueFilter
{
public:
    StateValueFilterAdaptive();

    void addValue(const QVariant &value) override;
    QVariant filteredValue() const override;
};

#endif // STATEVALUEFILTERADAPTIVE_H
