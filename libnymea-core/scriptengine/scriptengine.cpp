#include "scriptengine.h"
#include "devices/devicemanager.h"

#include <QQmlApplicationEngine>

namespace nymeaserver {

ScriptEngine::ScriptEngine(DeviceManager *deviceManager, QObject *parent) : QObject(parent),
    m_deviceManager(deviceManager)
{
    loadScripts();
}

void ScriptEngine::loadScripts()
{
    QString fileName = "/home/micha/Develop/nymea/tests/script.qml";
    QQmlApplicationEngine *engine = new QQmlApplicationEngine(this);

    engine->load(fileName);
}

}
