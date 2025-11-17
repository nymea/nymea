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

#include "i2cmanager.h"

/*! \fn QStringList I2CManager::availablePorts() const
        Lists the available I2C ports in the system.
 */

/*! \fn QList<I2CScanResult> I2CManager::scanRegisters(const QString &portName = QString())
        Scans the I2C systems for connected devices. If \a portName is given, only this port
        will be scanned. If \a portName is empty, all available ports in the system will be
        scanned.
 */

/*! \fn bool I2CManager::open(I2CDevice *i2cDevice)
        Open the given I2CDevice. After reimplementing an I2CDevice, this method can be used
        to open the device. Returns false if opening fails, for example because of missing
        permissions or if the given \ai2cDevice is not valid, for example because of a bad
        port name or slave address.
*/

/*! \fn bool I2CManager::startReading(I2CDevice *i2cDevice, int interval = 1000)
        Start reading from the given \a i2cDevice. When calling this, the I2CManager will start
        polling the given \a i2cDevice. Optionally, the interface can be given.
        Note that the interval might not be met, for example if the device is busy by other
        readers.
        The given \a i2cDevice is required to be opened first.
 */

/*! \fn void I2CManager::stopReading(I2CDevice *i2cDevice)
        Stops reading from the given \a i2cDevice.
 */

/*! \fn bool I2CManager::writeData(I2CDevice *i2cDevice, const QByteArray &data)
        Write the given \a data to the given \a i2cDevice.
        The data will be put into a write buffer and will be written to the device
        in a different thread when the i2c device is available.
        The given \a i2cDevice is required to be opened first.
 */

/*! \fn void I2CManager::close(I2CDevice *i2cDevice)
        Closes the giben \a i2cDevice. If reading are still active for this device,
        stopReading() will be called implicitly.
 */

I2CManager::I2CManager(QObject *parent):
    QObject(parent)
{

}
