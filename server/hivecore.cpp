#include "hivecore.h"
#include "jsonrpcserver.h"
#include "devicemanager.h"

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

HiveCore::HiveCore(QObject *parent) :
    QObject(parent)
{

    qDebug() << "creating devmanager";
    m_deviceManager = new DeviceManager(this);


    // start the server
    m_jsonServer = new JsonRPCServer(this);

}

void HiveCore::gotSignal(const QUuid &triggerId, const QVariantMap &params)
{
    qDebug() << "id: " << triggerId;
    qDebug() << params;

}
