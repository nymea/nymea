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

#ifndef RADIO433TRASMITTER_H
#define RADIO433TRASMITTER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDebug>

#include "libnymea.h"
#include "gpio.h"

namespace nymeaserver {

class Radio433Trasmitter : public QThread
{
    Q_OBJECT
public:
    explicit Radio433Trasmitter(QObject *parent = nullptr, int gpio = 22);
    ~Radio433Trasmitter();

    bool startTransmitter();
    bool available();

    void sendData(int delay, QList<int> rawData, int repetitions);

protected:
    void run();

private:
    int m_gpioPin;
    Gpio *m_gpio;

    QMutex m_mutex;
    bool m_enabled;

    QMutex m_allowSendingMutex;
    bool m_allowSending;

    QMutex m_queueMutex;
    QQueue<QList<int> > m_rawDataQueue;

    bool m_available;

    bool setUpGpio();

signals:

public slots:
    void allowSending(bool sending);

};

}

#endif // RADIO433TRASMITTER_H
