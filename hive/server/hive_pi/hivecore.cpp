#include "hivecore.h"

HiveCore::HiveCore(QObject *parent) :
    QObject(parent)
{
    m_server = new Server(this);
}
