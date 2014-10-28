/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

/*!
  \class Gpio
  \brief The Gpio class helps to interact with the gpio pins of the Raspberry Pi.

  \inmodule libguh

  The Raspberry Pi offers lower-level interfaces (GPIO's) intended to connect more directly
  with chips and subsystem modules. General Purpose Input/Output (a.k.a. GPIO) is a generic
  pin on a chip whose behavior (including whether it is an input or output pin) can be controlled
  through this class. An object of of the Gpio class represents a pin on the board.

  In following table is a list of all GPIO's of the Raspberry Pi Rev. 2.0:

  \image gpio.png "Gpio settings"

  Valid GPIO's for this class are those with a GPIO number (for example GPIO 22, which is on pin Nr. 15)

*/

#include "gpio.h"
#include <QDebug>

/*! Constructs a \l{Gpio} object to represent a GPIO with the given \a gpio number and the \a parent. */
Gpio::Gpio(QObject *parent, int gpio) :
    QObject(parent),m_gpio(gpio)
{
}

/*! Destroys the Gpio object and unexports the GPIO. */
Gpio::~Gpio()
{
    unexportGpio();
}

/*! Returns true if this GPIO could be exported in the system file "/sys/class/gpio/export". */
bool Gpio::exportGpio()
{
    unexportGpio();

    char buf[64];

    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        //qDebug() << "ERROR: could not open /sys/class/gpio/export";
        return false;
    }

    ssize_t len = snprintf(buf, sizeof(buf), "%d", m_gpio);
    if(write(fd, buf, len) != len){
        qDebug() << "ERROR: could not write to gpio (export)";
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

/*! Returns true if this GPIO could be unexported in the system file "/sys/class/gpio/unexport". */
bool Gpio::unexportGpio()
{
    char buf[64];

    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        //qDebug() << "ERROR: could not open /sys/class/gpio/unexport";
        return false;
    }

    ssize_t len = snprintf(buf, sizeof(buf), "%d", m_gpio);
    if(write(fd, buf, len) != len){
        //qDebug() << "ERROR: could not write to gpio (unexport)";
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

/*! Returns true, if the file of this GPIO could be opend.*/
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

/*! Returns true, if the direction \a dir of this GPIO could be set correctly.
 *
 * Possible directions are:
 *
 * \table
 * \header
 *      \li {2,1} Pin directions
 * \row
 *      \li 0
 *      \li INPUT
 * \row
 *      \li 1
 *      \li OUTPUT
 * \endtable
 */
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
        if(write(fd, "in", 3) != 3){
            qDebug() << "ERROR: could not write to gpio (set INPUT)";
            close(fd);
            return false;
        }
        m_dir = INPUT;
        close(fd);
        return true;
    }
    if(dir == OUTPUT){
        if(write(fd, "out", 4) != 4){
            qDebug() << "ERROR: could not write to gpio (set OUTPUT)";
            close(fd);
            return false;
        }
        m_dir = OUTPUT;
        close(fd);
        return true;
    }
    close(fd);
    return false;
}

/*! Returns true, if the digital \a value of the GPIO could be set correctly.
 *
 * Possible \a value 's are:
 *
 * \table
 * \header
 *      \li {2,1} Pin value
 * \row
 *      \li 0
 *      \li LOW
 * \row
 *      \li 1
 *      \li HIGH
 * \endtable
 */
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
            if(write(fd, "0", 2) != 2){
                qDebug() << "ERROR: could not write to gpio (set LOW)";
                close(fd);
                return false;
            }
            close(fd);
            return true;
        }
        if(value == HIGH){
            if(write(fd, "1", 2) != 2){
                qDebug() << "ERROR: could not write to gpio (set HIGH)";
                close(fd);
                return false;
            }
            close(fd);
            return true;
        }
        close(fd);
        return false;
    }else{
        qDebug() << "ERROR: Gpio" << m_gpio << "is not an OUTPUT.";
        return false;
    }
}

/*! Returns the current digital value of the GPIO.
 *
 * Possible values are:
 *
 * \table
 * \header
 *      \li {2,1} Pin directions
 * \row
 *      \li 0
 *      \li LOW
 * \row
 *      \li 1
 *      \li HIGH
 * \endtable
 */
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
    ssize_t len = read(fd, &ch, 1);
    if(len != 1){
        close(fd);
        return -1;
    }

    if (ch != '0') {
        value = 1;
    }else{
        value = 0;
    }
    close(fd);
    return value;
}

/*! Returns true, if the \a edge of this GPIO could be set correctly. The \a edge parameter specifies,
 *  when an interrupt occurs.
 *
 * Possible values are:
 *
 * \table
 * \header
 *      \li {2,1} Edge possibilitys
 * \row
 *      \li 0
 *      \li EDGE_FALLING
 * \row
 *      \li 1
 *      \li EDGE_RISING
 * \row
 *      \li 2
 *      \li EDGE_BOTH
 * \endtable
 */
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
        if(write(fd, "falling", 8) != 8){
            qDebug() << "ERROR: could not write to gpio (set EDGE_FALLING)";
            close(fd);
            return false;
        }
        close(fd);
        return true;
    }
    if(edge == EDGE_RISING){
        if(write(fd, "rising", 7) != 7){
            qDebug() << "ERROR: could not write to gpio (set EDGE_RISING)";
            close(fd);
            return false;
        }
        close(fd);
        return true;
    }
    if(edge == EDGE_BOTH){
        if(write(fd, "both", 5) != 5){
            qDebug() << "ERROR: could not write to gpio (set EDGE_BOTH)";
            close(fd);
            return false;
        }
        close(fd);
        return true;
    }
    close(fd);
    return false;
}

bool Gpio::setActiveLow(bool activeLow)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/active_low", m_gpio);
    int fd = open(buf, O_WRONLY);
    if (fd < 0) {
        qDebug() << "ERROR: could not open /sys/class/gpio" << m_gpio << "/active_low";
        return false;
    }

    if(activeLow){
        if(write(fd, "0", 2) != 2){
            qDebug() << "ERROR: could not write to gpio (set Active LOW)";
            close(fd);
            return false;
        }
        close(fd);
        return true;
    }
    if(!activeLow){
        if(write(fd, "1", 2) != 2){
            qDebug() << "ERROR: could not write to gpio (set Active HIGH)";
            close(fd);
            return false;
        }
        close(fd);
        return true;
    }
    close(fd);
    return false;
}

int Gpio::gpioPin()
{
    return m_gpio;
}

int Gpio::gpioDirection()
{
    return m_dir;
}

bool Gpio::isAvailable()
{
    QDir gpioDirectory("/sys/class/gpio");
    return gpioDirectory.exists();
}
