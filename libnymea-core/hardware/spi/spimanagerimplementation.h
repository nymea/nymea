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

#ifndef SPIMANAGERIMPLEMENTATION_H
#define SPIMANAGERIMPLEMENTATION_H

#include "hardware/spi/spimanager.h"

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QHash>
#include <QFuture>
#include <QFutureWatcher>
#include <QDateTime>

class QFile;

namespace nymeaserver {

class SPIManagerImplementation : public SPIManager
{
    Q_OBJECT
public:

    explicit SPIManagerImplementation(QObject *parent = nullptr);
    ~SPIManagerImplementation();

    QStringList availablePorts() const override;
    QList<SPIScanResult> scanRegisters(const QString &portName) override;

    bool open(SPIDevice *spiDevice) override;
    bool startReading(SPIDevice *spiDevice, int interval = 1000) override;
    void stopReading(SPIDevice *spiDevice) override;
    bool writeData(SPIDevice *spiDevice, const QByteArray &data) override;
    void close(SPIDevice *spiDevice) override;

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
        SPIDevice *device;
    };

    QMutex m_mutex;
    QHash<SPIDevice*, ReadingInfo> m_readers;
    QHash<SPIDevice*, QFile*> m_openFiles;

    QMutex m_writeQueueMutex;
    QList<WritingInfo> m_writeQueue;

    QFutureWatcher<void> m_watcher;

    QTimer m_pollTimer;

};

}

#endif // SPIMANAGERIMPLEMENTATION_H
