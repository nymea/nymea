/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef RADIO433_H
#define RADIO433_h

#include <QObject>
#include <QThread>
#include <gpio.h>

#define RC_MAX_CHANGES 67

class Radio433: public QObject
{
    Q_OBJECT

public:
    Radio433(QObject *parent = 0);
    ~Radio433();

public:
    void sendData(QList<int> rawData);

private:
    Gpio *m_receiver;
    Gpio *m_transmitter;

    unsigned int m_timings[RC_MAX_CHANGES];
    unsigned int m_duration;
    unsigned int m_changeCount;
    unsigned long m_lastTime;
    unsigned int m_repeatCount;
    unsigned int m_epochMicro;

    int micros();
    void delayMicros(int microSeconds);

private slots:
    void handleInterrupt();


signals:
    /*! This signal is emitted whenever a valid signal of 48 bits was recognized over the
     * 433 MHz receiver. The sync signal and the message are in the integer list \a rawData.
     */
    void dataReceived(QList<int> rawData);
};

#endif
