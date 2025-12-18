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

#include "i2cdevice.h"
#include "i2cmanager.h"

#include <QDebug>

/*! \fn QByteArray I2CDevice::readData(int fileDescriptor);
        Reimplement this when implementing reading communication with an I2C device.
        This method will be called repeatedly as soon as the I2CManager is instructed to start
        reading from this device. Read the current value from the device, e.g. a sensor reading
        and return the value.

        The given file descriptor will already be opened, the I2C slave address already be
        selected. The only task is to read the current value. Often that consists of a
        write operation to configure registers on the device followed by a read operation.

        IMPORTANT: This method will be called from a different thread. This means that you
        are free to perform blocking operations, including calling QThread::msleep() but it
        also implies that you must not access other members of the class if they may be
        accessed from code outside of this method. If you absolutely must do so, make sure
        to use mutexes (e.g. QMutex) accordingly.
*/

/*! \fn bool I2CDevice::writeData(int fileDescriptor, const QByteArray &data);
        Reimplement this when implementing writing communication with an I2C device.
        This method will be called when I2CManager::writeData(I2CDevice *device, const QByteArray &data)
        is called. The data to be written is copied and passed on to this method.

        The given file descriptor will already be opened, the I2C slave address already be
        selected. The only task is to write the data to it. Often that consists of a
        write operation to configure registers on the device followed by another write operation
        to write the actual data.

        IMPORTANT: This method will be called from a different thread. This means that you
        are free to perform blocking operations, including calling QThread::msleep() but it
        also implies that you must not access other members of the class if they may be
        accessed from code outside of this method. If you absolutely must do so, make sure
        to use mutexes (e.g. QMutex) accordingly.
*/

/*! Constructs an I2CDevice with the given portName and address. The \a portName
    must match the file name of /dev, for example "i2c-0" for /dev/i2c-0. The
    \a address describes the I2C slave address for this device.

    I2CManager::scanRegisters() can be used to scan for available I2C devices connected
    to the system.
*/
I2CDevice::I2CDevice(const QString &portName, int address, QObject *parent)
    : QObject(parent)
    , m_portName(portName)
    , m_address(address)
{}

I2CDevice::~I2CDevice() {}

/*! Returns the port name of this I2C device. */
QString I2CDevice::portName() const
{
    return m_portName;
}

/*! Returns the address of this I2C device. */
int I2CDevice::address() const
{
    return m_address;
}

QByteArray I2CDevice::readData(int fileDescriptor)
{
    Q_UNUSED(fileDescriptor)
    return QByteArray();
}

bool I2CDevice::writeData(int fileDescriptor, const QByteArray &data)
{
    Q_UNUSED(fileDescriptor)
    Q_UNUSED(data)
    return false;
}

QDebug operator<<(QDebug debug, const I2CDevice *i2cDevice)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "I2CDevice(Port: " << i2cDevice->portName() << ", Address: 0x" << QString::number(i2cDevice->address(), 16) << ")";
    return debug;
}
