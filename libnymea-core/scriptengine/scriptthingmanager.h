#ifndef SCRIPTTHINGMANAGER_H
#define SCRIPTTHINGMANAGER_H

#include "integrations/thingmanager.h"

#include <QObject>
#include <QQmlParserStatus>

namespace nymeaserver {
namespace scriptengine {

class ScriptThingManager : public QObject, public QQmlParserStatus
{
    Q_OBJECT
public:
    explicit ScriptThingManager(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;



signals:

private:
    ThingManager *m_thingManager = nullptr;
};

}
}

#endif // SCRIPTTHINGMANAGER_H
