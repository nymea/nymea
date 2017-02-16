/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.io>             *
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

#include "radio433transmitter.h"

Radio433Trasmitter::Radio433Trasmitter(QObject *parent, int gpio) :
    QThread(parent),m_gpioPin(gpio)
{
}

Radio433Trasmitter::~Radio433Trasmitter()
{
    quit();
    wait();
    deleteLater();
}

bool Radio433Trasmitter::startTransmitter()
{
    return setUpGpio();
}

bool Radio433Trasmitter::available()
{
    return m_available;
}

void Radio433Trasmitter::run()
{
    QList<int> rawData;

    m_queueMutex.lock();
    while(!m_rawDataQueue.isEmpty()){

        rawData = m_rawDataQueue.dequeue();
        m_queueMutex.unlock();

        m_gpio->setValue(Gpio::ValueLow);
        int flag=1;

        foreach (int delay, rawData) {
            // 1 = High, 0 = Low
            int value = flag %2;
            m_gpio->setValue((Gpio::Value)value);
            flag++;
            usleep(delay);
        }

        m_gpio->setValue(Gpio::ValueLow);
    }
    m_queueMutex.unlock();
}

void Radio433Trasmitter::allowSending(bool sending)
{
    m_allowSendingMutex.lock();
    m_allowSending = sending;
    m_allowSendingMutex.unlock();
}

bool Radio433Trasmitter::setUpGpio()
{
    m_gpio = new Gpio(m_gpioPin, this);

    if(!m_gpio->exportGpio() || !m_gpio->setDirection(Gpio::DirectionOutput) || !m_gpio->setValue(Gpio::ValueLow)){
        m_available = false;
        return false;
    }
    m_available = true;
    return true;
}

void Radio433Trasmitter::sendData(int delay, QList<int> rawData, int repetitions)
{
    QList<int> timings;
    for (int i = 0; i < repetitions; i++) {
        foreach (int data, rawData) {
            timings.append(delay*data);
        }
    }

    m_queueMutex.lock();
    m_rawDataQueue.enqueue(timings);
    m_queueMutex.unlock();

    if(!isRunning()){
        start();
    }
}
