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
    Q_PROPERTY(QString systemLocation READ systemLocation)
    Q_PROPERTY(bool backendSuggestionAvailable READ backendSuggestionAvailable)
    Q_PROPERTY(Zigbee::ZigbeeBackendType suggestedZigbeeBackendType READ suggestedZigbeeBackendType)
    Q_PROPERTY(qint32 suggestedBaudRate READ suggestedBaudRate)

public:
    explicit ZigbeeAdapter();

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    QString systemLocation() const;
    void setSystemLocation(const QString &systemLocation);

    bool backendSuggestionAvailable() const;
    void setBackendSuggestionAvailable(bool backendSuggestionAvailable);

    Zigbee::ZigbeeBackendType suggestedZigbeeBackendType() const;
    void setSuggestedZigbeeBackendType(Zigbee::ZigbeeBackendType backendType);

    qint32 suggestedBaudRate() const;
    void setSuggestedBaudRate(qint32 baudRate);

    bool operator==(const ZigbeeAdapter &other) const;

private:
    QString m_name;
    QString m_description;
    QString m_systemLocation;

    bool m_backendSuggestionAvailable = false;
    Zigbee::ZigbeeBackendType m_suggestedZigbeeBackendType = Zigbee::ZigbeeBackendTypeDeconz;
    qint32 m_suggestedBaudRate = 38400;
};

QDebug operator<<(QDebug debug, const ZigbeeAdapter &adapter);

}

Q_DECLARE_METATYPE(nymeaserver::ZigbeeAdapter)

#endif // ZIGBEEADAPTER_H

