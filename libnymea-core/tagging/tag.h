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

#ifndef TAG_H
#define TAG_H

#include "typeutils.h"

#include <QString>
#include <QVariant>

namespace nymeaserver {

class Tag
{
    Q_GADGET
    Q_PROPERTY(QString appId READ appId WRITE setAppId)
    Q_PROPERTY(QString tagId READ tagId WRITE setTagId)
    Q_PROPERTY(QUuid thingId READ thingId WRITE setThingId USER true)
    Q_PROPERTY(QUuid ruleId READ ruleId WRITE setRuleId USER true)
    Q_PROPERTY(QString value READ value WRITE setValue USER true)
public:
    Tag();
    Tag(const ThingId &thingId, const QString &appId, const QString &tagId, const QString &value);
    Tag(const RuleId &ruleId, const QString &appId, const QString &tagId, const QString &value);

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    RuleId ruleId() const;
    void setRuleId(const RuleId &ruleId);

    QString appId() const;
    void setAppId(const QString &appId);

    QString tagId() const;
    void setTagId(const QString &tagId);

    QString value() const;
    void setValue(const QString &value);

    bool operator==(const Tag &other) const;

private:
    ThingId m_thingId;
    RuleId m_ruleId;
    QString m_appId;
    QString m_tagId;
    QString m_value;
};

class Tags : public QList<Tag>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Tags();
    Tags(const QList<Tag> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

QDebug operator<<(QDebug dbg, const Tag &tag);
} // namespace nymeaserver

Q_DECLARE_METATYPE(nymeaserver::Tag)
Q_DECLARE_METATYPE(nymeaserver::Tags)

#endif // TAG_H
