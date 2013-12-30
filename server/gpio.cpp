#include "gpio.h"
#include <QDebug>

Gpio::Gpio(QObject *parent, int gpio) :
    QObject(parent),m_gpio(gpio)
{
    exportGpio();
}

Gpio::~Gpio()
{
    unexportGpio();
}

bool Gpio::exportGpio()
{
    char buf[64];

    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio/export";
        return false;
    }

    int len = snprintf(buf, sizeof(buf), "%d", m_gpio);
    write(fd, buf, len);
    close(fd);

    return true;
}

bool Gpio::unexportGpio()
{
    char buf[64];

    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio/unexport";
        return false;
    }
    int len = snprintf(buf, sizeof(buf), "%d", m_gpio);
    write(fd, buf, len);
    close(fd);

    return true;
}

int Gpio::openGpio()
{
    char buf[64];

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", m_gpio);

    int fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio" << m_gpio << "/direction";
        return fd;
    }
    return fd;
}

bool Gpio::setDirection(int dir)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", m_gpio);

    int fd = open(buf, O_WRONLY);
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio" << m_gpio << "/direction";
        return false;
    }
    if(dir == INPUT){
        write(fd, "in", 3);
        m_dir = INPUT;
        close(fd);
        return true;
    }
    if(dir == OUTPUT){
        write(fd, "out", 4);
        m_dir = OUTPUT;
        close(fd);
        return true;
    }
    close(fd);
    return false;
}

bool Gpio::setValue(unsigned int value)
{
    // check if gpio is a output
    if( m_dir == OUTPUT){
        char buf[64];
        snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", m_gpio);

        int fd = open(buf, O_WRONLY);
        if (fd < 0) {
            qDebug() << "ERROR: could not open /sys/class/gpio" << m_gpio << "/value";
            return false;
        }

        if(value == LOW){
            write(fd, "0", 2);
            close(fd);
            return true;
        }
        if(value == HIGH){
            write(fd, "1", 2);
            close(fd);
            return true;
        }
        close(fd);
        return false;
    }else{
        qDebug() << "ERROR: Gpio" << m_gpio << "is not a OUTPUT.";
        return false;
    }
}

int Gpio::getValue()
{
    char buf[64];

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", m_gpio);

    int fd = open(buf, O_RDONLY);
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio" << m_gpio << "/value";
        return -1;
    }

    char ch;
    int value = -1;

    read(fd, &ch, 1);

    if (ch != '0') {
        value = 1;
    }else{
        value = 0;
    }
    close(fd);

    qDebug() << "gpio" << m_gpio << "value = " << value;
    return value;
}

bool Gpio::setEdgeInterrupt(int edge)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/edge", m_gpio);

    int fd = open(buf, O_WRONLY);
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio" << m_gpio << "/edge";
        return false;
    }

    if(edge == EDGE_FALLING){
        write(fd, "falling", 8);
        close(fd);
        return true;
    }
    if(edge == EDGE_RISING){
        write(fd, "rising", 7);
        close(fd);
        return true;
    }
    if(edge == EDGE_BOTH){
        write(fd, "both", 5);
        close(fd);
        return true;
    }
    close(fd);
    return false;
}

void Gpio::enableInterrupt()
{
    struct pollfd fdset[2];
    int gpio_fd = openGpio();
    int nfds = 2;
    int rc;
    int timeout = 3000;
    char buf[64];

    while(1){
        memset((void*)fdset, 0, sizeof(fdset));
        fdset[0].fd = STDIN_FILENO;
        fdset[0].events = POLLIN;

        fdset[1].fd = gpio_fd;
        fdset[1].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0) {
            qDebug() << "ERROR: poll failed";
            return;
        }
        if(rc == 0){
            //timeout
        }
        if (fdset[1].revents & POLLPRI) {
            read(fdset[1].fd, buf, 64);
            emit pinInterrupt();
        }
    }
}
