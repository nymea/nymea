#include "gpiomonitor.h"

GpioMonitor::GpioMonitor(QObject *parent) :
    QThread(parent)
{
}

GpioMonitor::~GpioMonitor()
{
    foreach (Gpio* gpio, m_gpioList) {
        gpio->unexportGpio();
    }
    quit();
    wait();
    deleteLater();
}

void GpioMonitor::stop()
{
    m_enabledMutex.lock();
    m_enabled = false;
    m_enabledMutex.unlock();
}

bool GpioMonitor::addGpio(Gpio *gpio, bool activeLow)
{
    if (!gpio->exportGpio() || !gpio->setDirection(INPUT) || !gpio->setEdgeInterrupt(EDGE_BOTH) || !gpio->setActiveLow(activeLow)) {
        return false;
    }
    m_gpioListMutex.lock();
    m_gpioList.append(gpio);
    m_gpioListMutex.unlock();
    return true;
}

QList<Gpio *> GpioMonitor::gpioList()
{
    m_gpioListMutex.lock();
    QList<Gpio*> gpioList = m_gpioList;
    m_gpioListMutex.unlock();
    return gpioList;
}

void GpioMonitor::run()
{
    struct pollfd *fds;
    char val;
    int ret;
    int retVal;

    fds = (pollfd*) malloc(sizeof(pollfd) * m_gpioList.size());
    m_gpioListMutex.lock();
    for (int i = 0; i < m_gpioList.size(); i++) {
        fds[i].fd = m_gpioList[i]->openGpio();
        fds[i].events = POLLPRI | POLLERR;
    }
    m_gpioListMutex.unlock();

    bool enabled = true;

    m_enabledMutex.lock();
    m_enabled = true;
    m_enabledMutex.unlock();

    while (enabled) {
        m_gpioListMutex.lock();
        ret = poll(fds, m_gpioList.size(), 2000);

        if (ret > 0) {
            for (int i=0; i < m_gpioList.size(); i++) {
                if ((fds[i].revents & POLLPRI) || (fds[i].revents & POLLERR)) {
                    lseek(fds[i].fd, 0, SEEK_SET);
                    retVal = read(fds[i].fd, &val, 1);

                    emit changed(m_gpioList[i]->gpioPin(), m_gpioList[i]->getValue());

                    if (retVal < 0) {
                        qWarning() << "ERROR: poll failed";
                    }
                }
            }
        } else if (ret < 0) {
            qWarning() << "ERROR: poll failed: " << errno;
        }
        m_gpioListMutex.unlock();

        m_enabledMutex.lock();
        enabled = m_enabled;
        m_enabledMutex.unlock();
    }
    free(fds);
}
