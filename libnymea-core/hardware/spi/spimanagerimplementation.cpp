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

#include "spimanagerimplementation.h"

#include "hardware/spi/spidevice.h"
#include "loggingcategories.h"

#include <QDir>
#include <QtConcurrent/QtConcurrentRun>

#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>

#define SPI_SLAVE 0

namespace nymeaserver {

SPIManagerImplementation::SPIManagerImplementation(QObject *parent) : SPIManager(parent)
{
    m_pollTimer.setInterval(200);
    m_pollTimer.setSingleShot(true);
    connect(&m_pollTimer, &QTimer::timeout, this, &SPIManagerImplementation::nextCycle);
}

SPIManagerImplementation::~SPIManagerImplementation()
{
    m_watcher.waitForFinished();
}

QStringList nymeaserver::SPIManagerImplementation::availablePorts() const
{
    return QDir("/sys/class/spi-adapter/").entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
}

bool SPIManagerImplementation::open(SPIDevice *spiDevice)
{
    if (m_openFiles.contains(spiDevice)) {
        qCWarning(dcSPI()) << "SPI device" << spiDevice << "already opened.";
        return false;
    }

    QString fileName = "/dev/" + spiDevice->portName();

    foreach (SPIDevice *d, m_openFiles.keys()) {
        if (d->portName() == spiDevice->portName()) {
            // Another SPIDevice opened this file already. We'll hook into that.
            m_mutex.lock();
            m_openFiles.insert(spiDevice, m_openFiles.value(d));
            m_mutex.unlock();
            return true;
        }
    }

    if (!QFile::exists(fileName)) {
        qCWarning(dcSPI()) << "The SPI port does not exist:" << spiDevice->portName();
        return false;
    }

    QFile *file = new QFile("/dev/" + spiDevice->portName(), this);
    if (!file->open(QFile::ReadWrite)) {
        qCWarning(dcSPI()) << "Error opening SPI port" << spiDevice << "Error:" << file->errorString();
        delete file;
        return false;
    }

    m_mutex.lock();
    m_openFiles.insert(spiDevice, file);
    m_mutex.unlock();
    return true;
}

bool SPIManagerImplementation::startReading(SPIDevice *spiDevice, int interval)
{
    QMutexLocker locker(&m_mutex);

    if (!m_openFiles.contains(spiDevice)) {
        qCWarning(dcSPI()) << "SPIDevice not open. Cannot start reading.";
        return false;
    }
    qCDebug(dcSPI()) << "Starting to poll SPI device" << spiDevice;
    ReadingInfo readingInfo;
    readingInfo.interval = interval;
    m_readers.insert(spiDevice, readingInfo);

    if (!m_pollTimer.isActive()) {
        m_pollTimer.start();
    }
    return true;
}


void SPIManagerImplementation::stopReading(SPIDevice *spiDevice)
{
    QMutexLocker locker(&m_mutex);
    m_readers.remove(spiDevice);

    if (m_readers.count() == 0) {
        m_pollTimer.stop();
    }
}

bool SPIManagerImplementation::writeData(SPIDevice *spiDevice, const QByteArray &data)
{
    m_writeQueueMutex.lock();
    WritingInfo info;
    info.device = spiDevice;
    info.data = data;
    m_writeQueue.append(info);
    m_writeQueueMutex.unlock();
    return true;
}

void SPIManagerImplementation::close(SPIDevice *spiDevice)
{
    bool isInUse = false;
    m_mutex.lock();
    if (m_readers.contains(spiDevice)) {
        isInUse = true;
    }
    m_mutex.unlock();

    if (isInUse) {
        stopReading(spiDevice);
    }

    int refCount = 0;
    foreach (SPIDevice* d, m_openFiles.keys()) {
        if (d->portName() == spiDevice->portName()) {
            refCount++;
        }
    }
    if (refCount == 0) {

        m_mutex.lock();
        QFile *f = m_openFiles.take(spiDevice);
        m_mutex.unlock();

        f->close();
        f->deleteLater();
    }
}

void SPIManagerImplementation::nextCycle()
{
    QFuture<void> future = QtConcurrent::run([this](){
        // Copy the write queue to open it up as fast as possible for others to append new entries
        m_writeQueueMutex.lock();
        QList<WritingInfo> writeQueue = m_writeQueue;
        m_writeQueue.clear();
        m_writeQueueMutex.unlock();

        m_mutex.lock();

        foreach (const WritingInfo &info, writeQueue) {
            SPIDevice *spiDevice = info.device;

            int fd = m_openFiles.value(spiDevice)->handle();
            if (fd == -1) {
                qCWarning(dcSPI()) << "SPI device" << spiDevice << "not opened. Cannot write to it.";
                continue;
            }

            if (ioctl(fd, SPI_SLAVE, spiDevice->address()) < 0) {
                qCWarning(dcSPI()) << "Cannot select SPI slave address for SPI device" << spiDevice;
                continue;
            }

            qCDebug(dcSPI()) << "Writing to SPI device" << spiDevice;
            bool success = spiDevice->writeData(fd, info.data);

            QMetaObject::invokeMethod(spiDevice, "dataWritten", Qt::QueuedConnection, Q_ARG(bool, success));

        }

        foreach (SPIDevice *spiDevice, m_readers.keys()) {
            ReadingInfo readingInfo = m_readers.value(spiDevice);
            if (readingInfo.lastReading.addMSecs(readingInfo.interval) > QDateTime::currentDateTime()) {
                continue;
            }
            int fd = m_openFiles.value(spiDevice)->handle();
            if (fd == -1) {
                qCWarning(dcSPI()) << "SPI device" << spiDevice << "not opened. Cannot read.";
                continue;
            }

            if (ioctl(fd, SPI_SLAVE, spiDevice->address()) < 0) {
                qCWarning(dcSPI()) << "Cannot select SPI slave address for SPI device" << spiDevice;
                continue;
            }

            qCDebug(dcSPI()) << "Reading SPI device" << spiDevice;
            QByteArray data = spiDevice->readData(fd);

            m_readers[spiDevice].lastReading = QDateTime::currentDateTime();

            QMetaObject::invokeMethod(spiDevice, "readingAvailable", Qt::QueuedConnection, Q_ARG(QByteArray, data));
        }

        m_mutex.unlock();
    });

    m_watcher.setFuture(future);
    connect(&m_watcher, SIGNAL(finished()), &m_pollTimer, SLOT(start()));
}

}
