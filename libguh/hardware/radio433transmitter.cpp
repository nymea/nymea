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

#include "radio433transmitter.h"

Radio433Trasmitter::Radio433Trasmitter(QObject *parent, int gpio) :
    QThread(parent),m_gpioPin(gpio)
{
    m_gpio = new Gpio(this,m_gpioPin);

    stopTransmitter();
}

Radio433Trasmitter::~Radio433Trasmitter()
{
}

bool Radio433Trasmitter::startTransmitter()
{
    if(setUpGpio()){
        run();
        return true;
    }
    return false;
}

bool Radio433Trasmitter::stopTransmitter()
{
    m_mutex.lock();
    m_enabled = false;
    m_mutex.unlock();

    return true;
}

bool Radio433Trasmitter::setUpGpio()
{
    if(!m_gpio->exportGpio()){
        return false;
    }else{
        m_gpio->setDirection(OUTPUT);
        m_gpio->setValue(LOW);
    }
    return true;
}

void Radio433Trasmitter::sendData(QList<int> rawData)
{
    m_queueMutex.lock();
    m_rawDataQueue.enqueue(rawData);
    m_queueMutex.unlock();
}

void Radio433Trasmitter::run()
{
    bool enabled = true;

    m_mutex.lock();
    m_enabled = true;
    m_mutex.unlock();

    QList<int> rawData;

    while(enabled){

        m_queueMutex.lock();
        m_allowSendingMutex.lock();
        //check if we have data in the sending queue and we are allowed to send...
        if(!m_rawDataQueue.isEmpty() && m_allowSending){
            m_allowSendingMutex.unlock();

            rawData = m_rawDataQueue.dequeue();
            m_queueMutex.unlock();

            m_gpio->setValue(LOW);
            int flag=1;

            // send raw data 8 times
            for(int i = 0; i <= 8; i++){
                foreach (int delay, rawData) {
                    // 1 = High, 0 = Low
                    m_gpio->setValue(flag %2);
                    flag++;
                    usleep(delay);
                }

                //check if we can continue or we should stop
                m_allowSendingMutex.lock();
                if(!m_allowSending){
                    m_allowSendingMutex.unlock();
                    break;
                }
                m_allowSendingMutex.unlock();
            }
            m_gpio->setValue(LOW);
        }else{
            m_queueMutex.unlock();
            m_allowSendingMutex.unlock();
            break;
        }
        m_queueMutex.unlock();
        m_allowSendingMutex.unlock();

        m_mutex.lock();
        enabled = m_enabled;
        m_mutex.unlock();
    }
}

void Radio433Trasmitter::allowSending(bool sending)
{
    m_allowSendingMutex.lock();
    m_allowSending = sending;
    m_allowSendingMutex.unlock();
}
