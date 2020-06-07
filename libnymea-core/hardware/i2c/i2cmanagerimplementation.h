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

#ifndef I2CMANAGERIMPLEMENTATION_H
#define I2CMANAGERIMPLEMENTATION_H

#include "hardware/i2c/i2cmanager.h"

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QHash>
#include <QFuture>
#include <QFutureWatcher>
#include <QDateTime>

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
    class ReadingInfo {
    public:
        int interval;
        QDateTime lastReading;
    };
    class WritingInfo {
    public:
        QByteArray data;
        I2CDevice *device;
    };

    QMutex m_mutex;
    QHash<I2CDevice*, ReadingInfo> m_readers;
    QHash<I2CDevice*, QFile*> m_openFiles;

    QMutex m_writeQueueMutex;
    QList<WritingInfo> m_writeQueue;

    QFutureWatcher<void> m_watcher;

    QTimer m_pollTimer;

};

}

#endif // I2CMANAGERIMPLEMENTATION_H
