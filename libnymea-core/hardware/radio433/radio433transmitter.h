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
