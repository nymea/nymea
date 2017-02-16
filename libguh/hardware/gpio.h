/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.io>             *
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

#ifndef GPIO_H
#define GPIO_H

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "libguh.h"

class LIBGUH_EXPORT Gpio : public QObject
{
    Q_OBJECT
public:
    enum Direction {
        DirectionInput,
        DirectionOutput,
        DirectionInvalid
    };

    enum Value {
        ValueLow = 0,
        ValueHigh = 1,
        ValueInvalid = -1
    };

    enum Edge {
        EdgeFalling,
        EdgeRising,
        EdgeBoth,
        EdgeNone
    };

    explicit Gpio(const int &gpio, QObject *parent = 0);
    ~Gpio();

    QString gpioDirectory() const;
    int gpioNumber() const;

    static bool isAvailable();

    bool exportGpio();
    bool unexportGpio();

    bool setDirection(Gpio::Direction direction);
    Gpio::Direction direction();

    bool setValue(Gpio::Value value);
    Gpio::Value value();

    bool setActiveLow(bool activeLow);
    bool activeLow();

    bool setEdgeInterrupt(Gpio::Edge edge);
    Gpio::Edge edgeInterrupt();


private:
    int m_gpio;
    Gpio::Direction m_direction;
    QDir m_gpioDirectory;

};

QDebug operator<< (QDebug debug, Gpio *gpio);

#endif // GPIO_H
