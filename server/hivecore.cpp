#include "hivecore.h"
#include "jsonrpcserver.h"
#include "devicemanager.h"
#include "ruleengine.h"

#include <QDebug>

HiveCore* HiveCore::s_instance = 0;

HiveCore *HiveCore::instance()
{
    if (!s_instance) {
        s_instance = new HiveCore();
    }
    return s_instance;
}

DeviceManager *HiveCore::deviceManager() const
{
    return m_deviceManager;
}

RuleEngine *HiveCore::ruleEngine() const
{
    return m_ruleEngine;
}

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

    connect(m_deviceManager,SIGNAL(emitTrigger(QUuid,QVariantMap)),this,SLOT(gotSignal(QUuid,QVariantMap)));

}

void HiveCore::gotSignal(const QUuid &triggerId, const QVariantMap &params)
{
    qDebug() << "##################################################";
    qDebug() << "id: " << triggerId;
    qDebug() << params;

    foreach (const QUuid &actionId, m_ruleEngine->evaluateTrigger(triggerId)) {
        m_deviceManager->executeAction(actionId, params);
    }
}
