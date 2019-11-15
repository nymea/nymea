#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QUuid>
#include <QQmlEngine>

#include "devices/devicemanager.h"

namespace nymeaserver {

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    explicit ScriptEngine(DeviceManager *deviceManager, QObject *parent = nullptr);

signals:

private:
    void loadScripts();

private:
    DeviceManager *m_deviceManager = nullptr;
};

}

#endif // SCRIPTENGINE_H
