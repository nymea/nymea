/*!
    \class GuhCore
    \brief The main entry point for the Guh Server and the place where all the messages are dispatched.

    \ingroup core
    \inmodule server

    GuhCore is a singleton instance and the main entry point of the Guh daemon. It is responsible to
    instantiate, set up and connect all the other components.
*/

#include "guhcore.h"
#include "jsonrpcserver.h"
#include "devicemanager.h"
#include "ruleengine.h"

#include <QDebug>

GuhCore* GuhCore::s_instance = 0;

/*! Returns a pointer to the single \l{GuhCore} instance.*/
GuhCore *GuhCore::instance()
{
    if (!s_instance) {
        s_instance = new GuhCore();
    }
    return s_instance;
}

/*! Returns a pointer to the \l{DeviceManager} instance owned by GuhCore.*/
DeviceManager *GuhCore::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a pointer to the \l{RuleEngine} instance owned by GuhCore.*/
RuleEngine *GuhCore::ruleEngine() const
{
    return m_ruleEngine;
}

/*! Constructs GuhCore with the given \a parent. This is private.
    Use \l{GuhCore::instance()} to access the single instance.*/
GuhCore::GuhCore(QObject *parent) :
    QObject(parent)
{

    qDebug() << "*****************************************";
    qDebug() << "* Creating Device Manager               *";
    qDebug() << "*****************************************";
    m_deviceManager = new DeviceManager(this);

    qDebug() << "*****************************************";
    qDebug() << "* Creating Rule Engine                  *";
    qDebug() << "*****************************************";
    m_ruleEngine = new RuleEngine(this);

    qDebug() << "*****************************************";
    qDebug() << "* Starting JSON RPC Server              *";
    qDebug() << "*****************************************";
    m_jsonServer = new JsonRPCServer(this);

    connect(m_deviceManager, &DeviceManager::emitEvent, this, &GuhCore::gotSignal);

}

/*! Connected to the DeviceManager's emitEvent signal. Events received in
    here will be evaluated by the \l{RuleEngine} and the according \l{Action}{Actions} are executed.*/
void GuhCore::gotSignal(const Event &event)
{
    foreach (const Action &action, m_ruleEngine->evaluateEvent(event)) {
        m_deviceManager->executeAction(action);
    }
}
