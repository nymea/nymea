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

#ifndef ZIGBEEADAPTER_H
#define ZIGBEEADAPTER_H

#include <QObject>
#include <QDebug>

#include "zigbee.h"

namespace nymeaserver {

class ZigbeeAdapter
{
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(QString serialPort READ serialPort)
    Q_PROPERTY(QString serialNumber READ serialNumber)
    Q_PROPERTY(bool hardwareRecognized READ hardwareRecognized)
    Q_PROPERTY(QString backend READ backend)
    Q_PROPERTY(qint32 baudRate READ baudRate)

public:
    enum ZigbeeBackendType {
        ZigbeeBackendTypeDeconz,
        ZigbeeBackendTypeNxp,
#ifndef ZIGBEE_DISABLE_TI
        ZigbeeBackendTypeTi
#endif
    };
    Q_ENUM(ZigbeeBackendType)

    explicit ZigbeeAdapter();

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    QString serialPort() const;
    void setSerialPort(const QString &serialPort);

    QString serialNumber() const;
    void setSerialNumber(const QString &serialNumber);

    bool hardwareRecognized() const;
    void setHardwareRecognized(bool hardwareRecognized);

    ZigbeeAdapter::ZigbeeBackendType backendType() const;
    void setBackendType(ZigbeeAdapter::ZigbeeBackendType backendType);
    QString backend() const;

    qint32 baudRate() const;
    void setBaudRate(qint32 baudRate);

    bool operator==(const ZigbeeAdapter &other) const;

    static QHash<ZigbeeBackendType, QString> backendNames();

private:
    QString m_name;
    QString m_description;
    QString m_serialPort;
    QString m_serialNumber;
    bool m_hardwareRecognized = false;
    ZigbeeAdapter::ZigbeeBackendType m_backendType = ZigbeeAdapter::ZigbeeBackendTypeDeconz;
    qint32 m_baudRate = 38400;
};

QDebug operator<<(QDebug debug, const ZigbeeAdapter &adapter);

}

Q_DECLARE_METATYPE(nymeaserver::ZigbeeAdapter)
Q_DECLARE_METATYPE(nymeaserver::ZigbeeAdapter::ZigbeeBackendType)

#endif // ZIGBEEADAPTER_H

