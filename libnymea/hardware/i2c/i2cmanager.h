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
