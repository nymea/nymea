/*!
    \class HiveCore
    \brief The main entry point for the Hive Server and the place where all the messages are dispatched.

    \ingroup core
    \inmodule server

    HiveCore is a singleton instance and the main entry point of the Hive daemon. It is responsible to
    instantiate, set up and connect all the other components.
*/

#include "hivecore.h"
#include "jsonrpcserver.h"
#include "devicemanager.h"
#include "ruleengine.h"

#include <QDebug>

HiveCore* HiveCore::s_instance = 0;

/*! Returns a pointer to the single \l{HiveCore} instance.*/
HiveCore *HiveCore::instance()
{
    if (!s_instance) {
        s_instance = new HiveCore();
    }
    return s_instance;
}

/*! Returns a pointer to the \l{DeviceManager} instance owned by HiveCore.*/
DeviceManager *HiveCore::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a pointer to the \l{RuleEngine} instance owned by HiveCore.*/
RuleEngine *HiveCore::ruleEngine() const
{
    return m_ruleEngine;
}

/*! Constructs HiveCore with the given \a parent. This is private.
    Use \l{HiveCore::instance()} to access the single instance.*/
HiveCore::HiveCore(QObject *parent) :
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

    connect(m_deviceManager, &DeviceManager::emitTrigger, this, &HiveCore::gotSignal);

}

/*! Connected to the DeviceManager's emitTrigger signal. Triggers received in
    here will be evaluated by the \l{RuleEngine} and the according \l{Action}{Actions} are executed.*/
void HiveCore::gotSignal(const Trigger &trigger)
{
    foreach (const Action &action, m_ruleEngine->evaluateTrigger(trigger)) {
        m_deviceManager->executeAction(action);
    }
}
