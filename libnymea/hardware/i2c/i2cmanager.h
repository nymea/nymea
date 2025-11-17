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

#ifndef I2CMANAGER_H
#define I2CMANAGER_H

#include <QObject>

class I2CDevice;

struct I2CScanResult {
    QString portName;
    int address;
};

class I2CManager : public QObject
{
    Q_OBJECT
public:
    I2CManager(QObject *parent = nullptr);
    virtual ~I2CManager() = default;

    virtual QStringList availablePorts() const = 0;
    virtual QList<I2CScanResult> scanRegisters(const QString &portName = QString()) = 0;

    virtual bool open(I2CDevice *i2cDevice) = 0;
    virtual bool startReading(I2CDevice *i2cDevice, int interval = 1000) = 0;
    virtual void stopReading(I2CDevice *i2cDevice) = 0;
    virtual bool writeData(I2CDevice *i2cDevice, const QByteArray &data) = 0;
    virtual void close(I2CDevice *i2cDevice) = 0;
};

#endif // I2CMANAGER_H
