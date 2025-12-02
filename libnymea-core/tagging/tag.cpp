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

#include "tag.h"

#include <QDebug>

namespace nymeaserver {

Tag::Tag()
{

}

Tag::Tag(const ThingId &thingId, const QString &appId, const QString &tagId, const QString &value):
    m_thingId(thingId),
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

ThingId Tag::thingId() const
{
    return m_thingId;
}

void Tag::setThingId(const ThingId &thingId)
{
    m_thingId = thingId;
}

RuleId Tag::ruleId() const
{
    return m_ruleId;
}

void Tag::setRuleId(const RuleId &ruleId)
{
    m_ruleId = ruleId;
}

QString Tag::appId() const
{
    return m_appId;
}

void Tag::setAppId(const QString &appId)
{
    m_appId = appId;
}

QString Tag::tagId() const
{
    return m_tagId;
}

void Tag::setTagId(const QString &tagId)
{
    m_tagId = tagId;
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
    return m_thingId == other.thingId() &&
            m_ruleId == other.ruleId() &&
            m_appId == other.appId() &&
            m_tagId == other.tagId();
}

QDebug operator<<(QDebug dbg, const Tag &tag)
{
    QDebugStateSaver saver(dbg);
    if (!tag.thingId().isNull()) {
        dbg.nospace() << "Tag (ThingId:" << tag.thingId();
    } else {
        dbg.nospace() << "Tag (RuleId:" << tag.ruleId();
    }
    dbg.nospace() << ", AppId:" << tag.appId() << ", TagId:" << tag.tagId() << ", Value:" << tag.value() << ")" << '\n';
    return dbg;
}

Tags::Tags()
{

}

Tags::Tags(const QList<Tag> &other): QList<Tag>(other)
{

}

QVariant Tags::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Tags::put(const QVariant &variant)
{
    append(variant.value<Tag>());
}

}
