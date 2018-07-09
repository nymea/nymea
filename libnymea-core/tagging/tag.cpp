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

#include "tag.h"

#include <QDebug>

namespace nymeaserver {

Tag::Tag(const DeviceId &deviceId, const QString &appId, const QString &tagId, const QString &value):
    m_deviceId(deviceId),
    m_appId(appId),
    m_tagId(tagId),
    m_value(value)
{

}

Tag::Tag(const RuleId &ruleId, const QString &appId, const QString &tagId, const QString &value):
    m_ruleId(ruleId),
    m_appId(appId),
    m_tagId(tagId),
    m_value(value)

{

}

DeviceId Tag::deviceId() const
{
    return m_deviceId;
}

RuleId Tag::ruleId() const
{
    return m_ruleId;
}

QString Tag::appId() const
{
    return m_appId;
}

QString Tag::tagId() const
{
    return m_tagId;
}

QString Tag::value() const
{
    return m_value;
}

void Tag::setValue(const QString &value)
{
    m_value = value;
}

bool Tag::operator==(const Tag &other) const
{
    return m_deviceId == other.deviceId() &&
            m_ruleId == other.ruleId() &&
            m_appId == other.appId() &&
            m_tagId == other.tagId();
}

QDebug operator<<(QDebug dbg, const Tag &tag)
{
    if (!tag.deviceId().isNull()) {
        dbg.nospace() << "Tag (DeviceId:" << tag.deviceId();
    } else {
        dbg.nospace() << "Tag (RuleId:" << tag.ruleId();
    }
    dbg.nospace() << ", AppId:" << tag.appId() << ", TagId:" << tag.tagId() << ", Value:" << tag.value() << ")" << endl;
    return dbg;
}

}
