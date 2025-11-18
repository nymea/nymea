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

#ifndef RADIO433RECEIVER_H
#define RADIO433RECEIVER_H

#include <QThread>

#include "libnymea.h"
#include "../gpio.h"

class LIBGUH_EXPORT Radio433Receiver : public QThread
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

    bool startReceiver();
    bool stopReceiver();
    bool available();

protected:
    void run() override;

private:
    int m_gpioPin;
    Gpio *m_gpio;
    unsigned int m_epochMicro;

    unsigned int m_pulseProtocolOne;
    unsigned int m_pulseProtocolTwo;

    QList<int> m_timings;

    QMutex m_mutex;
    bool m_enabled;
    bool m_available;
    bool m_reading;

    bool setUpGpio();
    int micros();
    bool valueInTolerance(int value, int correctValue);
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
