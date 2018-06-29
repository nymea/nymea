/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TAG_H
#define TAG_H

#include "typeutils.h"

#include <QString>

namespace nymeaserver {

class Tag
{
public:
    Tag(const DeviceId &deviceId, const QString &appId, const QString &tagId, const QString &value);
    Tag(const RuleId &ruleId, const QString &appId, const QString &tagId, const QString &value);

    DeviceId deviceId() const;
    RuleId ruleId() const;
    QString appId() const;
    QString tagId() const;

    QString value() const;
    void setValue(const QString &value);

    bool operator==(const Tag &other) const;

private:
    DeviceId m_deviceId;
    RuleId m_ruleId;
    QString m_appId;
    QString m_tagId;
    QString m_value;
};

QDebug operator<<(QDebug dbg, const Tag &tag);
}

#endif // TAG_H
