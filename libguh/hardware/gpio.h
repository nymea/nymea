/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef GPIO_H
#define GPIO_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>

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
 *  Raspberry Pi Rev. 2.0 P1 Header         Raspberry Pi Rev. 2.0 P5 Header
 *   __________________________________      __________________________________
 *  |______________________|__________|     |______________________|__________|
 *  |   Name    |  PIN NR. | Function |     |   Name    |  PIN NR. | Function |
 *  |___________|__________|__________|     |___________|__________|__________|
 *  |  GPIO 2   |   3      |  SDA     |     |  GPIO 28  |   3      |  SDA     |
 *  |  GPIO 3   |   5      |  SCL     |     |  GPIO 29  |   4      |  SCL     |
 *  |  GPIO 4   |   7      |          |     |  GPIO 30  |   5      |          |
 *  |  GPIO 7   |   26     |  CE1     |     |  GPIO 31  |   6      |          |
 *  |  GPIO 8   |   24     |  CE0     |     |___________|__________|__________|
 *  |  GPIO 9   |   21     |  MISO    |
 *  |  GPIO 10  |   19     |  MOSI    |
 *  |  GPIO 11  |   23     |  SCLK    |
 *  |  GPIO 14  |   8      |  TXD     |
 *  |  GPIO 15  |   10     |  RXD     |
 *  |  GPIO 17  |   11     |          |
 *  |  GPIO 18  |   12     |  PCM_CLK |
 *  |  GPIO 22  |   15     |          |
 *  |  GPIO 23  |   16     |          |
 *  |  GPIO 24  |   18     |          |
 *  |  GPIO 25  |   22     |          |
 *  |  GPIO 27  |   13     |          |
 *  |___________|__________|__________|
 *
 **********************************
 */

class Gpio : public QObject
{
    Q_OBJECT
public:
    explicit Gpio(QObject *parent = 0, int gpio = 0);
    ~Gpio();

    bool exportGpio();
    bool unexportGpio();

    int openGpio();
    bool setDirection(int dir);
    int getDirection();

    bool setValue(unsigned int value);
    int getValue();

    bool setEdgeInterrupt(int edge);
    bool setActiveLow(bool activeLow);

    int gpioNumber();
    bool isAvailable();

private:
    int m_gpio;
    int m_dir;

};

#endif // GPIO_H
