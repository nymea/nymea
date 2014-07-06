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
  \class Radio433
  \brief The Radio433 class helps to interact with the 433 MHz Receiver and Transmitter.

  \inmodule libguh

*/

#include "radio433.h"

Radio433::Radio433(QObject *parent) :
    QObject(parent)
{
    m_receiver = new Radio433Receiver();
    m_transmitter = new Radio433Trasmitter();

    connect(m_receiver,SIGNAL(readingChanged(bool)),this,SLOT(readingChanged(bool)));

    m_receiver->startReceiver();
    m_transmitter->startTransmitter();
}

Radio433::~Radio433()
{
    m_receiver->wait();
    m_receiver->quit();

    m_transmitter->wait();
    m_transmitter->quit();
}

void Radio433::readingChanged(bool reading)
{
    if(reading){
        m_transmitter->allowSending(false);
    }else{
        m_transmitter->allowSending(true);
    }
}

void Radio433::sendData(QList<int> rawData)
{
    m_transmitter->sendData(rawData);
}
