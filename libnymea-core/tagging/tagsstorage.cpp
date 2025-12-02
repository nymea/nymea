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

#include "tagsstorage.h"
#include "integrations/thingmanager.h"
#include "ruleengine/ruleengine.h"
#include "nymeasettings.h"

namespace nymeaserver {

TagsStorage::TagsStorage(ThingManager *thingManager, RuleEngine *ruleEngine, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager),
    m_ruleEngine(ruleEngine)
{
    connect(thingManager, &ThingManager::thingRemoved, this, &TagsStorage::thingRemoved);
    connect(ruleEngine, &RuleEngine::ruleRemoved, this, &TagsStorage::ruleRemoved);

    NymeaSettings settings(NymeaSettings::SettingsRoleTags);

    if (settings.childGroups().contains("Things")) {
        settings.beginGroup("Things");
    } else { // backwards compatibility with <= 0.19
        settings.beginGroup("Devices");
    }

    foreach (const QString &thingId, settings.childGroups()) {
        settings.beginGroup(thingId);
        foreach (const QString &appId, settings.childGroups()) {
            settings.beginGroup(appId);
            foreach (const QString &tagId, settings.childKeys()) {
                Tag tag(ThingId(thingId), appId, tagId, settings.value(tagId).toString());
                m_tags.append(tag);
            }
            settings.endGroup();
        }
        settings.endGroup();
    }
    settings.endGroup();

    // Migration path from nymea <= 0.19
    if (settings.childGroups().contains("Devices")) {
        // Save all Devices tags to things tags and drop Devices group
        foreach (const Tag &tag, m_tags) {
            saveTag(tag);
        }
        settings.remove("Devices");
    }

    settings.beginGroup("Rules");
    foreach (const QString &ruleId, settings.childGroups()) {
        settings.beginGroup(ruleId);
        foreach (const QString &appId, settings.childGroups()) {
            settings.beginGroup(appId);
            foreach (const QString &tagId, settings.childKeys()) {
                Tag tag(RuleId(ruleId), appId, tagId, settings.value(tagId).toString());
                m_tags.append(tag);
            }
            settings.endGroup();
        }
        settings.endGroup();
    }
    settings.endGroup();
}

QList<Tag> TagsStorage::tags() const
{
    return m_tags;
}

QList<Tag> TagsStorage::tags(const ThingId &thingId) const
{
    QList<Tag> ret;
    foreach (const Tag &tag, m_tags) {
        if (tag.thingId() == thingId) {
            ret.append(tag);
        }
    }
    return ret;
}

QList<Tag> TagsStorage::tags(const RuleId &ruleId) const
{
    QList<Tag> ret;
    foreach (const Tag &tag, m_tags) {
        if (tag.ruleId() == ruleId) {
            ret.append(tag);
        }
    }
    return ret;
}

TagsStorage::TagError TagsStorage::addTag(const Tag &tag)
{
    if (!tag.thingId().isNull()) {
        if (!m_thingManager->findConfiguredThing(tag.thingId())) {
            return TagsStorage::TagErrorThingNotFound;
        }
    } else if (!tag.ruleId().isNull()) {
       if (!m_ruleEngine->findRule(tag.ruleId()).isValid()) {
           return TagsStorage::TagErrorRuleNotFound;
       }
    }

    int index = m_tags.indexOf(tag);
    if (index >= 0) {
        m_tags.replace(index, tag);
        emit tagValueChanged(tag);
    } else {
        m_tags.append(tag);
        emit tagAdded(tag);
    }
    saveTag(tag);
    return TagsStorage::TagErrorNoError;
}

TagsStorage::TagError TagsStorage::removeTag(const Tag &tag)
{
    if (!m_tags.contains(tag)) {
        return TagErrorTagNotFound;
    }
    m_tags.removeAll(tag);
    unsaveTag(tag);
    emit tagRemoved(tag);
    return TagErrorNoError;
}


void TagsStorage::thingRemoved(const ThingId &thingId)
{
    QList<Tag> tagsToRemove;
    foreach (const Tag &tag, m_tags) {
        if (tag.thingId() == thingId) {
            tagsToRemove.append(tag);
        }
    }
    while (!tagsToRemove.isEmpty()) {
        removeTag(tagsToRemove.takeFirst());
    }
}

void TagsStorage::ruleRemoved(const RuleId &ruleId)
{
    QList<Tag> tagsToRemove;
    foreach (const Tag &tag, m_tags) {
        if (tag.ruleId() == ruleId) {
            tagsToRemove.append(tag);
        }
    }
    while (!tagsToRemove.isEmpty()) {
        removeTag(tagsToRemove.takeFirst());
    }
}

void TagsStorage::saveTag(const Tag &tag)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleTags);

    if (!tag.thingId().isNull()) {
        settings.beginGroup("Things");
        settings.beginGroup(tag.thingId().toString());
    } else {
        settings.beginGroup("Rules");
        settings.beginGroup(tag.ruleId().toString());
    }
    settings.beginGroup(tag.appId());
    settings.setValue(tag.tagId(), tag.value());
}

void TagsStorage::unsaveTag(const Tag &tag)
{
    NymeaSettings settings(NymeaSettings::SettingsRoleTags);

    if (!tag.thingId().isNull()) {
        settings.beginGroup("Things");
        settings.beginGroup(tag.thingId().toString());
    } else {
        settings.beginGroup("Rules");
        settings.beginGroup(tag.thingId().toString());
    }
    settings.beginGroup(tag.appId());
    settings.remove(tag.tagId());
}

}
