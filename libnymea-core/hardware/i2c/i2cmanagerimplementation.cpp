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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "i2cmanagerimplementation.h"

#include "hardware/i2c/i2cdevice.h"
#include "loggingcategories.h"

#include <QDir>
#include <QtConcurrent/QtConcurrentRun>

#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>

namespace nymeaserver {

I2CManagerImplementation::I2CManagerImplementation(QObject *parent) : I2CManager(parent)
{
    m_pollTimer.setInterval(200);
    m_pollTimer.setSingleShot(true);
    connect(&m_pollTimer, &QTimer::timeout, this, &I2CManagerImplementation::nextCycle);
}

I2CManagerImplementation::~I2CManagerImplementation()
{
    m_watcher.waitForFinished();
}

QStringList nymeaserver::I2CManagerImplementation::availablePorts() const
{
    return QDir("/sys/class/i2c-adapter/").entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
}

QList<I2CScanResult> nymeaserver::I2CManagerImplementation::scanRegisters(const QString &portName)
{
    QList<I2CScanResult> ret;

    QList<QString> portsToBeScanned = {portName};
    if (portName.isEmpty()) {
        portsToBeScanned = availablePorts();
    }

    m_mutex.lock();

    foreach (const QString &p, portsToBeScanned) {
        QFile f("/dev/" + p);
        if (!f.open(QFile::ReadWrite)) {
            qCWarning(dcI2C()) << "Failed to open I2C port" << p << "for scanning";
            continue;
        }

        for (int address = 0x03; address <= 0x77; address++) {
            // First check if selecting the slave address is possible at all
            if (ioctl(f.handle(), I2C_SLAVE, address) >= 0) {
                char probe = 0x00;
                long res = 0;
                // This is how the kernels i2cdetect scans:
                // Try to read from address 0x30 - 0x35 and 0x50 to 0x5F and write to the others.
                if ((address >= 0x30 && address <= 0x37)
                        || (address >= 0x50 && address <= 0x5F)) {
                    res  = read(f.handle(), &probe, 1);
                } else {
                    res = write(f.handle(), &probe, 1);
                }
                if (res == 1) {
                    qCDebug(dcI2C()) << QString("Found slave device at address 0x%1").arg(address, 0, 16);
                    I2CScanResult result;
                    result.portName = p;
                    result.address = address;
                    ret.append(result);
                }
            }
        }
    }
    m_mutex.unlock();
    return ret;
}

bool I2CManagerImplementation::open(I2CDevice *i2cDevice)
{
    if (m_openFiles.contains(i2cDevice)) {
        qCWarning(dcI2C()) << "I2C device" << i2cDevice << "already opened.";
        return false;
    }

    QString fileName = "/dev/" + i2cDevice->portName();

    foreach (I2CDevice *d, m_openFiles.keys()) {
        if (d->portName() == i2cDevice->portName()) {
            // Another I2CDevice opened this file already. We'll hook into that.
            m_mutex.lock();
            m_openFiles.insert(i2cDevice, m_openFiles.value(d));
            m_mutex.unlock();
            return true;
        }
    }

    if (!QFile::exists(fileName)) {
        qCWarning(dcI2C()) << "The I2C port does not exist:" << i2cDevice->portName();
        return false;
    }

    QFile *file = new QFile("/dev/" + i2cDevice->portName(), this);
    if (!file->open(QFile::ReadWrite)) {
        qCWarning(dcI2C()) << "Error opening I2C port" << i2cDevice << "Error:" << file->errorString();
        delete file;
        return false;
    }

    m_mutex.lock();
    m_openFiles.insert(i2cDevice, file);
    m_mutex.unlock();
    return true;
}

bool I2CManagerImplementation::startReading(I2CDevice *i2cDevice, int interval)
{
    QMutexLocker locker(&m_mutex);

    if (!m_openFiles.contains(i2cDevice)) {
        qCWarning(dcI2C()) << "I2CDevice not open. Cannot start reading.";
        return false;
    }
    qCDebug(dcI2C()) << "Starting to poll I2C device" << i2cDevice;
    ReadingInfo readingInfo;
    readingInfo.interval = interval;
    m_readers.insert(i2cDevice, readingInfo);

    if (!m_pollTimer.isActive()) {
        m_pollTimer.start();
    }
    return true;
}


void I2CManagerImplementation::stopReading(I2CDevice *i2cDevice)
{
    QMutexLocker locker(&m_mutex);
    m_readers.remove(i2cDevice);

    if (m_readers.count() == 0) {
        m_pollTimer.stop();
    }
}

bool I2CManagerImplementation::writeData(I2CDevice *i2cDevice, const QByteArray &data)
{
    m_writeQueueMutex.lock();
    WritingInfo info;
    info.device = i2cDevice;
    info.data = data;
    m_writeQueue.append(info);
    m_writeQueueMutex.unlock();
    return true;
}

void I2CManagerImplementation::close(I2CDevice *i2cDevice)
{
    bool isInUse = false;
    m_mutex.lock();
    if (m_readers.contains(i2cDevice)) {
        isInUse = true;
    }
    m_mutex.unlock();

    if (isInUse) {
        stopReading(i2cDevice);
    }

    int refCount = 0;
    foreach (I2CDevice* d, m_openFiles.keys()) {
        if (d->portName() == i2cDevice->portName()) {
            refCount++;
        }
    }
    if (refCount == 0) {

        m_mutex.lock();
        QFile *f = m_openFiles.take(i2cDevice);
        m_mutex.unlock();

        f->close();
        f->deleteLater();
    }
}

void I2CManagerImplementation::nextCycle()
{
    QFuture<void> future = QtConcurrent::run([this](){
        // Copy the write queue to open it up as fast as possible for others to append new entries
        m_writeQueueMutex.lock();
        QList<WritingInfo> writeQueue = m_writeQueue;
        m_writeQueue.clear();
        m_writeQueueMutex.unlock();

        m_mutex.lock();

        foreach (const WritingInfo &info, writeQueue) {
            I2CDevice *i2cDevice = info.device;

            int fd = m_openFiles.value(i2cDevice)->handle();
            if (fd == -1) {
                qCWarning(dcI2C()) << "I2C device" << i2cDevice << "not opened. Cannot write to it.";
                continue;
            }

            if (ioctl(fd, I2C_SLAVE, i2cDevice->address()) < 0) {
                qCWarning(dcI2C()) << "Cannot select I2C slave address for I2C device" << i2cDevice;
                continue;
            }

            qCDebug(dcI2C()) << "Writing to I2C device" << i2cDevice;
            bool success = i2cDevice->writeData(fd, info.data);

            QMetaObject::invokeMethod(i2cDevice, "dataWritten", Qt::QueuedConnection, Q_ARG(bool, success));

        }

        foreach (I2CDevice *i2cDevice, m_readers.keys()) {
            ReadingInfo readingInfo = m_readers.value(i2cDevice);
            if (readingInfo.lastReading.addMSecs(readingInfo.interval) > QDateTime::currentDateTime()) {
                continue;
            }
            int fd = m_openFiles.value(i2cDevice)->handle();
            if (fd == -1) {
                qCWarning(dcI2C()) << "I2C device" << i2cDevice << "not opened. Cannot read.";
                continue;
            }

            if (ioctl(fd, I2C_SLAVE, i2cDevice->address()) < 0) {
                qCWarning(dcI2C()) << "Cannot select I2C slave address for I2C device" << i2cDevice;
                continue;
            }

            qCDebug(dcI2C()) << "Reading I2C device" << i2cDevice;
            QByteArray data = i2cDevice->readData(fd);

            m_readers[i2cDevice].lastReading = QDateTime::currentDateTime();

            QMetaObject::invokeMethod(i2cDevice, "readingAvailable", Qt::QueuedConnection, Q_ARG(QByteArray, data));
        }

        m_mutex.unlock();
    });

    m_watcher.setFuture(future);
    connect(&m_watcher, &QFutureWatcher<void>::finished, this, [this](){
        m_pollTimer.start();
    });
}

}
