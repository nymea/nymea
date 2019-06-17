#ifndef BROWSERITEM_H
#define BROWSERITEM_H

#include "libnymea.h"
#include "typeutils.h"

#include <QList>

class LIBNYMEA_EXPORT BrowserItem
{
public:
    BrowserItem();
};


class LIBNYMEA_EXPORT BrowserItems: public QList<BrowserItem>
{

};

#endif // BROWSERITEM_H
