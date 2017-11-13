/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RADIO433_H
#define RADIO433_H

#include <QObject>

#include "libguh.h"
#include "hardwareresource.h"
#include "radio433brennenstuhlgateway.h"

class LIBGUH_EXPORT Radio433 : public HardwareResource
{
    Q_OBJECT

    friend class HardwareManager;

private:
    explicit Radio433(QObject *parent = 0);
    Radio433BrennenstuhlGateway *m_brennenstuhlTransmitter;

private slots:
    void brennenstuhlAvailableChanged(const bool &available);

public slots:
    bool sendData(int delay, QList<int> rawData, int repetitions);

    bool enable();
    bool disable();

};

#endif // RADIO433_H

