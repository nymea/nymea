#include "scriptthingmanager.h"

#include <qqml.h>
#include <QQmlEngine>

namespace nymeaserver {
namespace scriptengine {

ScriptThingManager::ScriptThingManager(QObject *parent)
    : QObject{parent}
{

}

void ScriptThingManager::classBegin()
{
    m_thingManager = reinterpret_cast<ThingManager*>(qmlEngine(this)->property("thingManager").toULongLong());

}

}
}
