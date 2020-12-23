#ifndef STATEVALUEFILTER_H
#define STATEVALUEFILTER_H

#include <QVariant>

class StateValueFilter
{
public:
    StateValueFilter();
    virtual ~StateValueFilter();

    virtual void addValue(const QVariant &value) = 0;

    virtual QVariant filteredValue() const = 0;

};

#endif // STATEVALUEFILTER_H
