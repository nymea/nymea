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

#include "tagshandler.h"

#include "nymeacore.h"
#include "tagging/tagsstorage.h"

namespace nymeaserver {

TagsHandler::TagsHandler(QObject *parent) : JsonHandler(parent)
{
    // Enums
    registerEnum<TagsStorage::TagError>();

    // Objects
    QVariantMap tag;
    tag.insert("o:deviceId", enumValueName(Uuid));
    tag.insert("o:ruleId", enumValueName(Uuid));
    tag.insert("appId", enumValueName(String));
    tag.insert("tagId", enumValueName(String));
    tag.insert("o:value", enumValueName(String));
    registerObject("Tag", tag);

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the Tags matching the given filter. Tags can be filtered by a deviceID, a ruleId, an appId, a tagId or a combination of any (however, combining deviceId and ruleId will return an empty result set).";
    params.insert("o:deviceId", enumValueName(Uuid));
    params.insert("o:ruleId", enumValueName(Uuid));
    params.insert("o:appId", enumValueName(String));
    params.insert("o:tagId", enumValueName(String));
    returns.insert("tagError", enumRef<TagsStorage::TagError>());
    returns.insert("o:tags", QVariantList() << objectRef("Tag"));
    registerMethod("GetTags", description, params, returns);

    params.clear(); returns.clear();
    description = "Add a Tag. A Tag must have a deviceId OR a ruleId (call this method twice if you want to attach the same tag to a device and a rule), an appId (Use the appId of your app), a tagId (e.g. \"favorites\") and a value. Upon success, a TagAdded notification will be emitted. Calling this method twice for the same ids (device/rule, appId and tagId) but with a different value will update the tag's value and the TagValueChanged notification will be emitted.";
    params.insert("tag", objectRef("Tag"));
    returns.insert("tagError", enumRef<TagsStorage::TagError>());
    registerMethod("AddTag", description, params, returns);

    params.clear(); returns.clear();
    description = "Remove a Tag. Tag value is optional and will be disregarded. If the ids match, the tag will be deleted and a TagRemoved notification will be emitted.";
    params.insert("tag", objectRef("Tag"));
    returns.insert("tagError", enumRef<TagsStorage::TagError>());
    registerMethod("RemoveTag", description, params, returns);

    // Notifications
    params.clear();
    description = "Emitted whenever a tag is added to the system. ";
    params.insert("tag", objectRef("Tag"));
    registerNotification("TagAdded", description, params);
    connect(NymeaCore::instance()->tagsStorage(), &TagsStorage::tagAdded, this, &TagsHandler::onTagAdded);

    params.clear();
    description = "Emitted whenever a tag is removed from the system. ";
    params.insert("tag", objectRef("Tag"));
    registerNotification("TagRemoved", description, params);
    connect(NymeaCore::instance()->tagsStorage(), &TagsStorage::tagRemoved, this, &TagsHandler::onTagRemoved);

    params.clear();
    description = "Emitted whenever a tag's value is changed in the system. ";
    params.insert("tag", objectRef("Tag"));
    registerNotification("TagValueChanged", description, params);
    connect(NymeaCore::instance()->tagsStorage(), &TagsStorage::tagValueChanged, this, &TagsHandler::onTagValueChanged);
}

QString TagsHandler::name() const
{
    return "Tags";
}

JsonReply *TagsHandler::GetTags(const QVariantMap &params) const
{
    QVariantList ret;
    foreach (const Tag &tag, NymeaCore::instance()->tagsStorage()->tags()) {
        if (params.contains("deviceId") && params.value("deviceId").toString() != tag.deviceId().toString()) {
            continue;
        }
        if (params.contains("ruleId") && params.value("ruleId").toString() != tag.ruleId().toString()) {
            continue;
        }
        if (params.contains("appId") && params.value("appId").toString() != tag.appId()) {
            continue;
        }
        if (params.contains("tagId") && params.value("tagId").toString() != tag.tagId()) {
            continue;
        }
        ret.append(packTag(tag));
    }
    QVariantMap returns = statusToReply(TagsStorage::TagErrorNoError);
    returns.insert("tags", ret);
    return createReply(returns);

}

JsonReply *TagsHandler::AddTag(const QVariantMap &params) const
{
    Tag tag = unpackTag(params.value("tag").toMap());
    TagsStorage::TagError error = NymeaCore::instance()->tagsStorage()->addTag(tag);
    QVariantMap returns = statusToReply(error);
    return createReply(returns);
}

JsonReply *TagsHandler::RemoveTag(const QVariantMap &params) const
{
    Tag tag = unpackTag(params.value("tag").toMap());
    TagsStorage::TagError error = NymeaCore::instance()->tagsStorage()->removeTag(tag);
    QVariantMap returns = statusToReply(error);
    return createReply(returns);
}

void TagsHandler::onTagAdded(const Tag &tag)
{
    qCDebug(dcJsonRpc) << "Notify \"Tags.TagAdded\"";
    QVariantMap params;
    params.insert("tag", packTag(tag));
    emit TagAdded(params);
}

void TagsHandler::onTagRemoved(const Tag &tag)
{
    qCDebug(dcJsonRpc) << "Notify \"Tags.TagRemoved\"";
    QVariantMap params;
    params.insert("tag", packTag(tag));
    emit TagRemoved(params);
}

void TagsHandler::onTagValueChanged(const Tag &tag)
{
    qCDebug(dcJsonRpc) << "Notify \"Tags.TagValueChanged\"";
    QVariantMap params;
    params.insert("tag", packTag(tag));
    emit TagValueChanged(params);
}

QVariantMap TagsHandler::packTag(const Tag &tag)
{
    QVariantMap ret;
    if (!tag.deviceId().isNull()){
        ret.insert("deviceId", tag.deviceId().toString());
    } else {
        ret.insert("ruleId", tag.ruleId().toString());
    }
    ret.insert("appId", tag.appId());
    ret.insert("tagId", tag.tagId());
    ret.insert("value", tag.value());
    return ret;
}

Tag TagsHandler::unpackTag(const QVariantMap &tagMap)
{
    DeviceId deviceId = DeviceId(tagMap.value("deviceId").toString());
    RuleId ruleId = RuleId(tagMap.value("ruleId").toString());
    QString appId = tagMap.value("appId").toString();
    QString tagId = tagMap.value("tagId").toString();
    QString value = tagMap.value("value").toString();
    if (!deviceId.isNull()) {
        return Tag(deviceId, appId, tagId, value);
    }
    return Tag(ruleId, appId, tagId, value);
}

QVariantMap TagsHandler::statusToReply(TagsStorage::TagError status) const
{
    QVariantMap returns;
    returns.insert("tagError", enumValueName<TagsStorage::TagError>(status));
    return returns;
}

}
