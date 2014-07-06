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

#ifndef RADIO433_H
#define RADIO433_H

#include <QObject>

#include "radio433receiver.h"
#include "radio433transmitter.h"

class Radio433 : public QObject
{
    Q_OBJECT
public:
    explicit Radio433(QObject *parent = 0);
    ~Radio433();

private:
    Radio433Receiver *m_receiver;
    Radio433Trasmitter *m_transmitter;

signals:
    void dataReceived(QList<int> rawData);

private slots:
    void readingChanged(bool reading);

public slots:
    void sendData(QList<int> rawData);

};

#endif // RADIO433_H

