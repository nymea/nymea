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

QString ZigbeeAdapter::systemLocation() const
{
    return m_systemLocation;
}

void ZigbeeAdapter::setSystemLocation(const QString &systemLocation)
{
    m_systemLocation = systemLocation;
}

bool ZigbeeAdapter::backendSuggestionAvailable() const
{
    return m_backendSuggestionAvailable;
}

void ZigbeeAdapter::setBackendSuggestionAvailable(bool backendSuggestionAvailable)
{
    m_backendSuggestionAvailable = backendSuggestionAvailable;
}

Zigbee::ZigbeeBackendType ZigbeeAdapter::suggestedZigbeeBackendType() const
{
    return m_suggestedZigbeeBackendType;
}

void ZigbeeAdapter::setSuggestedZigbeeBackendType(Zigbee::ZigbeeBackendType backendType)
{
    m_suggestedZigbeeBackendType = backendType;
}

qint32 ZigbeeAdapter::suggestedBaudRate() const
{
    return m_suggestedBaudRate;
}

void ZigbeeAdapter::setSuggestedBaudRate(qint32 baudRate)
{
    m_suggestedBaudRate = baudRate;
}

bool ZigbeeAdapter::operator==(const ZigbeeAdapter &other) const
{
    return m_systemLocation == other.systemLocation()
            && m_name == other.name()
            && m_description == other.description()
            && m_backendSuggestionAvailable == other.backendSuggestionAvailable()
            && m_suggestedZigbeeBackendType == other.suggestedZigbeeBackendType()
            && m_suggestedBaudRate == other.suggestedBaudRate();
}


QDebug operator<<(QDebug debug, const ZigbeeAdapter &adapter)
{
    debug.nospace() << "ZigbeeAdapter(" << adapter.name() << " - " << adapter.description();
    debug.nospace() << ", " << adapter.systemLocation();
    if (adapter.backendSuggestionAvailable()) {
        debug.nospace() << "Suggested backend: " << adapter.suggestedZigbeeBackendType();
        debug.nospace() << ", " << adapter.suggestedBaudRate();
    }

    debug.nospace() << ")";
    return debug.space();
}

}
