/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef RADIO433TRASMITTER_H
#define RADIO433TRASMITTER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDebug>

#include "libguh.h"
#include "../gpio.h"


class LIBGUH_EXPORT Radio433Trasmitter : public QThread
{
    Q_OBJECT
public:
    explicit Radio433Trasmitter(QObject *parent = 0, int gpio = 22);
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
#endif // RADIO433TRASMITTER_H
