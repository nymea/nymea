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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zigbeeadapter.h"

namespace nymeaserver {

ZigbeeAdapter::ZigbeeAdapter()
{

}

QString ZigbeeAdapter::name() const
{
    return m_name;
}

void ZigbeeAdapter::setName(const QString &name)
{
    m_name = name;
}

QString ZigbeeAdapter::description() const
{
    return m_description;
}

void ZigbeeAdapter::setDescription(const QString &description)
{
    m_description = description;
}

QString ZigbeeAdapter::serialPort() const
{
    return m_serialPort;
}

void ZigbeeAdapter::setSerialPort(const QString &serialPort)
{
    m_serialPort = serialPort;
}

QString ZigbeeAdapter::serialNumber() const
{
    return m_serialNumber;
}

void ZigbeeAdapter::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

bool ZigbeeAdapter::hardwareRecognized() const
{
    return m_hardwareRecognized;
}

void ZigbeeAdapter::setHardwareRecognized(bool hardwareRecognized)
{
    m_hardwareRecognized = hardwareRecognized;
}

ZigbeeAdapter::ZigbeeBackendType ZigbeeAdapter::backendType() const
{
    return m_backendType;
}

void ZigbeeAdapter::setBackendType(ZigbeeAdapter::ZigbeeBackendType backendType)
{
    m_backendType = backendType;
}

QString ZigbeeAdapter::backend() const
{
    return backendNames().value(m_backendType);
}

qint32 ZigbeeAdapter::baudRate() const
{
    return m_baudRate;
}

void ZigbeeAdapter::setBaudRate(qint32 baudRate)
{
    m_baudRate = baudRate;
}

bool ZigbeeAdapter::operator==(const ZigbeeAdapter &other) const
{
    return m_serialPort == other.serialPort()
            && m_name == other.name()
            && m_description == other.description()
            && m_hardwareRecognized == other.hardwareRecognized()
            && m_backendType == other.backendType()
            && m_baudRate == other.baudRate();
}

QHash<ZigbeeAdapter::ZigbeeBackendType, QString> ZigbeeAdapter::backendNames()
{
    QHash<ZigbeeAdapter::ZigbeeBackendType, QString> backendNameHash;
    backendNameHash.insert(ZigbeeBackendTypeDeconz, "deCONZ");
    backendNameHash.insert(ZigbeeBackendTypeNxp, "NXP");
    backendNameHash.insert(ZigbeeBackendTypeTi, "TI");
    return backendNameHash;
}

QDebug operator<<(QDebug debug, const ZigbeeAdapter &adapter)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ZigbeeAdapter(" << adapter.name() << " - " << adapter.description();
    debug.nospace() << ", " << adapter.serialPort();
    if (adapter.hardwareRecognized()) {
        debug.nospace() << " Hardware recognized: " << adapter.backendType();
        debug.nospace() << ", " << adapter.baudRate();
    }

    debug.nospace() << ")";
    return debug;
}

}
