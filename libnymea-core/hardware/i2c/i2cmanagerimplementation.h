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

#ifndef I2CMANAGERIMPLEMENTATION_H
#define I2CMANAGERIMPLEMENTATION_H

#include "hardware/i2c/i2cmanager.h"

#include <QDateTime>
#include <QFuture>
#include <QFutureWatcher>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QTimer>

class QFile;

namespace nymeaserver {

class I2CManagerImplementation : public I2CManager
{
    Q_OBJECT
public:
    explicit I2CManagerImplementation(QObject *parent = nullptr);
    ~I2CManagerImplementation();

    QStringList availablePorts() const override;
    QList<I2CScanResult> scanRegisters(const QString &portName) override;

    bool open(I2CDevice *i2cDevice) override;
    bool startReading(I2CDevice *i2cDevice, int interval = 1000) override;
    void stopReading(I2CDevice *i2cDevice) override;
    bool writeData(I2CDevice *i2cDevice, const QByteArray &data) override;
    void close(I2CDevice *i2cDevice) override;

private slots:
    void nextCycle();

private:
    class ReadingInfo
    {
    public:
        int interval;
        QDateTime lastReading;
    };
    class WritingInfo
    {
    public:
        QByteArray data;
        I2CDevice *device;
    };

    QMutex m_mutex;
    QHash<I2CDevice *, ReadingInfo> m_readers;
    QHash<I2CDevice *, QFile *> m_openFiles;

    QMutex m_writeQueueMutex;
    QList<WritingInfo> m_writeQueue;

    QFutureWatcher<void> m_watcher;

    QTimer m_pollTimer;
};

} // namespace nymeaserver

#endif // I2CMANAGERIMPLEMENTATION_H
