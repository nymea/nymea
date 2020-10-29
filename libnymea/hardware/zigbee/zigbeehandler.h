#ifndef ZIGBEEHANDLER_H
#define ZIGBEEHANDLER_H

#include "zigbeenode.h"

class ZigbeeHandler
{
public:
    ZigbeeHandler();
    virtual ~ZigbeeHandler() = default;

    virtual QString name() const = 0;
    virtual bool handleNode(ZigbeeNode *node, const QUuid &networkUuid) = 0;
};

#endif // ZIGBEEHANDLER_H
