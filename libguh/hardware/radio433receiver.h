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

#ifndef RADIO433RECEIVER_H
#define RADIO433RECEIVER_H

#include <QThread>

#include "gpio.h"

class Radio433Receiver : public QThread
{
    Q_OBJECT
public:
    explicit Radio433Receiver(QObject *parent = 0, int gpio = 27);
    ~Radio433Receiver();

    enum Protocol{
        Protocol48,
        Protocol64,
        ProtocolNone
    };

    void startReceiver();
    void stopReceiver();

private:
    int m_gpioPin;
    Gpio *m_gpio;
    unsigned int m_epochMicro;

    unsigned int m_pulseProtocolOne;
    unsigned int m_pulseProtocolTwo;

    QList<int> m_timings;

    QMutex m_mutex;
    bool m_enabled;
    bool m_reading;

    void run();
    bool setUpGpio();
    int micros();
    bool valueInTolerance(int value, int sollValue);
    bool checkValue(int value);
    bool checkValues(Protocol protocol);
    void changeReading(bool reading);

private slots:
    void handleTiming(int duration);

signals:
    void timingReady(int duration);
    void dataReceived(QList<int> rawData);
    void readingChanged(const bool &reading);

public slots:
};

#endif // RADIO433RECEIVER_H
