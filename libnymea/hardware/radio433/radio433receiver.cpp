// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QFile>
#include <QFileSystemWatcher>
#include <QDebug>
#include <sys/time.h>

#include "radio433receiver.h"
#include "loggingcategories.h"

Radio433Receiver::Radio433Receiver(QObject *parent, int gpio) :
    QThread(parent),m_gpioPin(gpio)
{
    m_timings.clear();
    m_available = false;

    m_mutex.lock();
    m_enabled = false;
    m_mutex.unlock();

    connect(this, &Radio433Receiver::timingReady, this, &Radio433Receiver::handleTiming, Qt::DirectConnection);
}

Radio433Receiver::~Radio433Receiver()
{
    quit();
    wait();
    deleteLater();
}

bool Radio433Receiver::stopReceiver()
{
    m_mutex.lock();
    m_enabled = false;
    m_mutex.unlock();
    return true;
}

bool Radio433Receiver::available()
{
    return m_available;
}

void Radio433Receiver::run()
{
    struct pollfd fdset[2];
    int gpio_fd = m_gpio->openGpio();
    char buf[1];

    bool enabled = true;
    m_mutex.lock();
    m_enabled = true;
    m_mutex.unlock();

    int lastTime = micros();
    // poll the gpio file, if something changes...emit the signal with the current pulse length
    while(enabled){
        memset((void*)fdset, 0, sizeof(fdset));
        fdset[0].fd = STDIN_FILENO;
        fdset[0].events = POLLIN;

        fdset[1].fd = gpio_fd;
        fdset[1].events = POLLPRI;
        int rc = poll(fdset, 2, 1000);

        if (rc < 0) {
            qCWarning(dcHardware) << "ERROR: poll failed";
            return;
        }
        if(rc == 0){
            //timeout
        }
        if (fdset[1].revents & POLLPRI){
            if(read(fdset[1].fd, buf, 1) != 1){
                qCWarning(dcHardware) << "could not read GPIO";
            }
            int currentTime = micros();
            int duration = currentTime - lastTime;
            lastTime = currentTime;
            emit timingReady(duration);
        }

        m_mutex.lock();
        enabled = m_enabled;
        m_mutex.unlock();
    }
}

bool Radio433Receiver::setUpGpio()
{
    if(!m_gpio){
        m_gpio = new Gpio(this,m_gpioPin);
    }

    if(!m_gpio->exportGpio() || !m_gpio->setDirection(INPUT) || !m_gpio->setEdgeInterrupt(EDGE_BOTH)){
        return false;
    }
    return true;
}

bool Radio433Receiver::startReceiver()
{
    if(!setUpGpio()){
        m_available = false;
        return false;
    }

    m_mutex.lock();
    m_enabled = true;
    m_mutex.unlock();
    m_available = true;

    start();
    return true;
}

int Radio433Receiver::micros()
{
    struct timeval tv ;
    int now ;
    gettimeofday (&tv, NULL) ;
    now  = (int)tv.tv_sec * (int)1000000 + (int)tv.tv_usec ;

    return (int)(now - m_epochMicro) ;
}

bool Radio433Receiver::valueInTolerance(int value, int correctValue)
{
    // in in range of +- 200 [us] of sollValue return true, eles return false
    if(value >= (double)correctValue - 200 && value <= (double)correctValue + 200){
        return true;
    }
    return false;
}

bool Radio433Receiver::checkValue(int value)
{
    if(valueInTolerance(value,m_pulseProtocolOne) || valueInTolerance(value,2*m_pulseProtocolOne) || valueInTolerance(value,3*m_pulseProtocolOne) || valueInTolerance(value,4*m_pulseProtocolOne) || valueInTolerance(value,8*m_pulseProtocolOne)){
        return true;
    }
    if(valueInTolerance(value,m_pulseProtocolTwo) || valueInTolerance(value,2*m_pulseProtocolTwo) || valueInTolerance(value,3*m_pulseProtocolTwo) || valueInTolerance(value,4*m_pulseProtocolTwo) || valueInTolerance(value,8*m_pulseProtocolTwo)){
        return true;
    }
    return false;
}

bool Radio433Receiver::checkValues(Protocol protocol)
{
    switch (protocol) {
    case Protocol48:
        for(int i = 1; i < m_timings.count(); i++){
            if(!(valueInTolerance(m_timings.at(i),m_pulseProtocolOne) || valueInTolerance(m_timings.at(i),2*m_pulseProtocolOne) || valueInTolerance(m_timings.at(i),3*m_pulseProtocolOne) || valueInTolerance(m_timings.at(i),4*m_pulseProtocolOne) || valueInTolerance(m_timings.at(i),8*m_pulseProtocolOne))){
                break;
            }
        }
        return true;
    case Protocol64:
        for(int i = 1; i < m_timings.count(); i++){
            if(!(valueInTolerance(m_timings.at(i),m_pulseProtocolTwo) || valueInTolerance(m_timings.at(i),2*m_pulseProtocolTwo) || valueInTolerance(m_timings.at(i),3*m_pulseProtocolTwo) || valueInTolerance(m_timings.at(i),4*m_pulseProtocolTwo) || valueInTolerance(m_timings.at(i),8*m_pulseProtocolTwo))){
                break;
            }
        }
        return true;
    default:
        break;
    }
    return false;
}

void Radio433Receiver::changeReading(bool reading)
{
    if(reading != m_reading){
        m_reading = reading;

        //TODO: create colission detection

        //emit readingChanged(reading);
    }
}

void Radio433Receiver::handleTiming(int duration)
{
    // to short...
    if(duration < 60){
        changeReading(false);
        m_timings.clear();
        return;
    }

    // could by a sync signal...
    bool sync = false;
    if(duration > 2400 && duration < 14000){
        changeReading(false);
        sync = true;
    }

    // got sync signal and list is not empty...
    if(!m_timings.isEmpty() && sync){
        // 1 sync bit + 48 data bit
        if(m_timings.count() == 49 && checkValues(Protocol48)){
            //qCWarning(dcHardware) << "48 bit ->" << m_timings << "\n--------------------------";
            changeReading(false);
            emit dataReceived(m_timings);
        }

        // 1 sync bit + 64 data bit
        if(m_timings.count() == 65 && checkValues(Protocol64)){
            //qCWarning(dcHardware) << "64 bit ->" << m_timings << "\n--------------------------";
            changeReading(false);
            emit dataReceived(m_timings);
        }
        m_timings.clear();
        m_pulseProtocolOne = 0;
        m_pulseProtocolTwo = 0;
        changeReading(false);
    }

    // got sync signal and list is empty...
    if(m_timings.isEmpty() && sync){
        m_timings.append(duration);
        m_pulseProtocolOne = (int)((double)m_timings.first()/31);
        m_pulseProtocolTwo = (int)((double)m_timings.first()/10);
        changeReading(false);
        return;
    }

    // list not empty and this is a possible value
    if(!m_timings.isEmpty() && checkValue(duration)){
        m_timings.append(duration);

        // check if it could be a signal -> if we have a sync and 15 valid values
        // set reading true to prevent a collision from own transmitter
        if(m_timings.count() == 20 && (checkValues(Protocol48) || checkValues(Protocol64))){
            changeReading(true);
        }

        // check if we have already a vallid protocol
        // 1 sync bit + 48 data bit
        if(m_timings.count() == 49 && checkValues(Protocol48)){
            //qCWarning(dcHardware) << "48 bit -> " << m_timings << "\n--------------------------";
            emit dataReceived(m_timings);
        }

        // 1 sync bit + 64 data bit
        if(m_timings.count() == 65 && checkValues(Protocol64)){
            //qCWarning(dcHardware) << "64 bit -> " << m_timings << "\n--------------------------";
            changeReading(false);
            emit dataReceived(m_timings);
            m_timings.clear();
        }
    }
}
