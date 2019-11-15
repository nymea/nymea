#include "scriptengine.h"
#include "devices/devicemanager.h"

#include "scriptaction.h"
#include "scriptevent.h"
#include "scriptstate.h"

#include <QQmlApplicationEngine>

namespace nymeaserver {

ScriptEngine::ScriptEngine(DeviceManager *deviceManager, QObject *parent) : QObject(parent),
    m_deviceManager(deviceManager)
{
    qmlRegisterType<ScriptEvent>("nymea", 1, 0, "Event");
    qmlRegisterType<ScriptAction>("nymea", 1, 0, "Action");
    qmlRegisterType<ScriptState>("nymea", 1, 0, "State");

    loadScripts();
}

void ScriptEngine::loadScripts()
{
    QString fileName = "/home/micha/Develop/nymea/tests/script.qml";

    QQmlApplicationEngine *engine = new QQmlApplicationEngine(this);
    engine->setProperty("deviceManager", reinterpret_cast<quint64>(m_deviceManager));

    engine->load(fileName);
}

}
