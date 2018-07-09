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

#include "tagsstorage.h"
#include "devicemanager.h"
#include "ruleengine.h"
#include "nymeasettings.h"

namespace nymeaserver {

TagsStorage::TagsStorage(DeviceManager *deviceManager, RuleEngine *ruleEngine, QObject *parent):
    QObject(parent),
    m_deviceManager(deviceManager),
    m_ruleEngine(ruleEngine)
{
    connect(deviceManager, &DeviceManager::deviceRemoved, this, &TagsStorage::deviceRemoved);

    NymeaSettings settings(NymeaSettings::SettingsRoleTags);

    settings.beginGroup("Devices");
    foreach (const QString &deviceId, settings.childGroups()) {
        settings.beginGroup(deviceId);
        foreach (const QString &appId, settings.childGroups()) {
            settings.beginGroup(appId);
            foreach (const QString &tagId, settings.childKeys()) {
                Tag tag(DeviceId(deviceId), appId, tagId, settings.value(tagId).toString());
                m_tags.append(tag);
            }
            settings.endGroup();
        }
        settings.endGroup();
    }
    settings.endGroup();
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

QList<Tag> TagsStorage::tags(const DeviceId &deviceId) const
{
    QList<Tag> ret;
    foreach (const Tag &tag, m_tags) {
        if (tag.deviceId() == deviceId) {
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
    if (!tag.deviceId().isNull()) {
        if (!m_deviceManager->findConfiguredDevice(tag.deviceId())) {
            return TagsStorage::TagErrorDeviceNotFound;
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

void TagsStorage::deviceRemoved(const DeviceId &deviceId)
{
    QList<Tag> tagsToRemove;
    foreach (const Tag &tag, m_tags) {
        if (tag.deviceId() == deviceId) {
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

    if (!tag.deviceId().isNull()) {
        settings.beginGroup("Devices");
        settings.beginGroup(tag.deviceId().toString());
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

    if (!tag.deviceId().isNull()) {
        settings.beginGroup("Devices");
        settings.beginGroup(tag.deviceId().toString());
    } else {
        settings.beginGroup("Rules");
        settings.beginGroup(tag.deviceId().toString());
    }
    settings.beginGroup(tag.appId());
    settings.remove(tag.tagId());
}

}
