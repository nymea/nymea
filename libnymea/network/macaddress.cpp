/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#include "macaddress.h"

#include <net/ethernet.h>

#include <QDebug>
#include <QRegularExpression>

MacAddress::MacAddress()
{
    clear();
}

MacAddress::MacAddress(const QString &macAddress)
{
    QString addressString;

    // Filter out any non hex characters from the string (like any separators)
    QRegularExpression hexMatcher("^[0-9A-F]", QRegularExpression::CaseInsensitiveOption);
    for (int i = 0; i < macAddress.count(); i++) {
        // Remove all possible separators
        QRegularExpressionMatch match = hexMatcher.match(macAddress.at(i));
        if (match.hasMatch()) {
            addressString.append(macAddress.at(i));
        }
    }

    // The remaining hex value has to be 12 characters (6 hex values)
    if (addressString.size() == 12) {
        m_rawData = QByteArray::fromHex(addressString.toUtf8());
    } else {
        // Make invalid and null
        m_rawData.clear();
    }
}

MacAddress::MacAddress(const QByteArray &macAddress)
{
    m_rawData = macAddress;
}

MacAddress::MacAddress(unsigned char *rawData)
{
    clear();
    for (int i = 0; i < ETH_ALEN; i++) {
        m_rawData[i] = rawData[i];
    }
}

MacAddress::MacAddress(const MacAddress &other)
{
    m_rawData = other.toByteArray();
}

MacAddress MacAddress::broadcast()
{
    return MacAddress(QByteArray::fromHex("ffffffffffff"));
}

MacAddress MacAddress::fromString(const QString &macAddress)
{
    return MacAddress(macAddress);
}

QString MacAddress::toString() const
{
    QString macString(QStringLiteral("%1:%2:%3:%4:%5:%6"));
    QByteArray data = m_rawData;
    if (!isValid()) {
        data = QByteArray(ETH_ALEN, '\0');
    }

    for (int i = 0; i < data.size(); i++) {
        macString = macString.arg(static_cast<quint8>(data.at(i)), 2, 16, QLatin1Char('0'));
    }

    return macString.toLower();
}

QByteArray MacAddress::toByteArray() const
{
    return m_rawData;
}

bool MacAddress::isNull() const
{
    if (!isValid())
        return true;

    return m_rawData == QByteArray(ETH_ALEN, '\0');
}

bool MacAddress::isValid() const
{
    return m_rawData.size() == ETH_ALEN;
}

void MacAddress::clear()
{
    m_rawData.fill('\0', ETH_ALEN);
}

MacAddress &MacAddress::operator=(const MacAddress &other)
{
    m_rawData = other.toByteArray();
    return *this;
}

bool MacAddress::operator<(const MacAddress &other) const
{
    return m_rawData < other.toByteArray();
}

bool MacAddress::operator>(const MacAddress &other) const
{
    return m_rawData > other.toByteArray();
}

bool MacAddress::operator==(const MacAddress &other) const
{
    return m_rawData == other.toByteArray();
}

bool MacAddress::operator!=(const MacAddress &other) const
{
    return m_rawData != other.toByteArray();
}

QDebug operator<<(QDebug debug, const MacAddress &macAddress)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "MacAddress(" << macAddress.toString() << ")";
    return debug;
}
