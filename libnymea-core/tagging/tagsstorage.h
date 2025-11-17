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

#ifndef TAGSSTORAGE_H
#define TAGSSTORAGE_H

#include "tag.h"
#include "typeutils.h"

#include <QObject>
#include <QVector>

class ThingManager;

namespace nymeaserver {

class RuleEngine;

class TagsStorage : public QObject
{
    Q_OBJECT
public:
    enum TagError {
        TagErrorNoError,
        TagErrorThingNotFound,
        TagErrorRuleNotFound,
        TagErrorTagNotFound
    };
    Q_ENUM(TagError)

    explicit TagsStorage(ThingManager* thingManager, RuleEngine* ruleEngine, QObject *parent = nullptr);

    TagError addTag(const Tag &tag);
    TagError removeTag(const Tag &tag);

    QList<Tag> tags() const;
    QList<Tag> tags(const ThingId &thingId) const;
    QList<Tag> tags(const RuleId &ruleId) const;

signals:
    void tagAdded(const Tag &tag);
    void tagRemoved(const Tag &tag);
    void tagValueChanged(const Tag &tag);

private slots:
    void thingRemoved(const ThingId &thingId);
    void ruleRemoved(const RuleId &ruleId);

private:
    void saveTag(const Tag &tag);
    void unsaveTag(const Tag &tag);

private:
    ThingManager *m_thingManager;
    RuleEngine *m_ruleEngine;
    QList<Tag> m_tags;
};

}

#endif // TAGSSTORAGE_H
