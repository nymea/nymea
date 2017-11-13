#include "plugintimer.h"
#include "loggingcategories.h"

PluginTimer::PluginTimer(int intervall, QObject *parent) :
    HardwareResource(HardwareResource::TypeTimer, "Plugin timer", parent),
    m_intervall(intervall)
{
    // FIXME: the timer should be able to emit timerEvents with different resolutions
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(m_intervall);

    connect(m_timer, &QTimer::timeout, this, &PluginTimer::timerEvent);
    setAvailable(true);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

bool PluginTimer::enable()
{
    if (!available())
        return false;

    m_timer->start();
    setEnabled(true);
    return true;
}

bool PluginTimer::disable()
{
    if (!available())
        return false;

    m_timer->stop();
    setEnabled(false);
    return true;
}
