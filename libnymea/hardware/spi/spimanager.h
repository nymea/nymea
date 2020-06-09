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

#ifndef SPIMANAGER_H
#define SPIMANAGER_H

#include <QObject>

class SPIDevice;

struct SPIScanResult {
    QString portName;
    int address;
};

class SPIManager : public QObject
{
    Q_OBJECT
public:
    SPIManager(QObject *parent = nullptr);
    virtual ~SPIManager() = default;

    virtual QStringList availablePorts() const = 0;
    virtual QList<SPIScanResult> scanRegisters(const QString &portName = QString()) = 0;

    virtual bool open(SPIDevice *spiDevice) = 0;
    virtual bool startReading(SPIDevice *spiDevice, int interval = 1000) = 0;
    virtual void stopReading(SPIDevice *spiDevice) = 0;
    virtual bool writeData(SPIDevice *spiDevice, const QByteArray &data) = 0;
    virtual void close(SPIDevice *spiDevice) = 0;
};

#endif // SPIMANAGER_H
