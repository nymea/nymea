#ifndef GPIO_H
#define GPIO_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define     INPUT   0
#define     OUTPUT  1
#define     LOW     0
#define     HIGH    1

#define     EDGE_FALLING    0
#define     EDGE_RISING     1
#define     EDGE_BOTH       2

/**********************************
 *   ______________________
 *  |______________________|
 *  |  File NR. |  PIN NR. |
 *  |___________|__________|
 *  |  GPIO 2   |   3      |
 *  |  GPIO 3   |   5      |
 *  |  GPIO 4   |   7      |
 *  |  GPIO 7   |   26     |
 *  |  GPIO 8   |   24     |
 *  |  GPIO 9   |   21     |
 *  |  GPIO 10  |   19     |
 *  |  GPIO 11  |   23     |
 *  |  GPIO 14  |   8      |
 *  |  GPIO 15  |   10     |
 *  |  GPIO 17  |   11     |
 *  |  GPIO 18  |   12     |
 *  |  GPIO 22  |   15     |
 *  |  GPIO 23  |   16     |
 *  |  GPIO 24  |   18     |
 *  |  GPIO 25  |   22     |
 *  |  GPIO 27  |   13     |
 *  |___________|__________|
 *
 **********************************
 */

class Gpio : public QThread
{
    Q_OBJECT
public:
    explicit Gpio(QObject *parent = 0, int gpio = 0);
    ~Gpio();

    void run() override;

    bool exportGpio();
    bool unexportGpio();

    int openGpio();

    bool setDirection(int dir);

    bool setValue(unsigned int value);
    int getValue();

    bool setEdgeInterrupt(int edge);

    void stop();



private:
    int m_gpio;
    int m_dir;
    QMutex m_mutex;
    bool m_enabled;

signals:
    void pinInterrupt();

public slots:

};

#endif // GPIO_H
