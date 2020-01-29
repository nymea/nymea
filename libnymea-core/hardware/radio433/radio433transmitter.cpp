/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "radio433transmitter.h"

namespace nymeaserver {

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

}
