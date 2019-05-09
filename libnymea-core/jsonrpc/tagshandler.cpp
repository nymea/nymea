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
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetTags", "Get the Tags matching the given filter. Tags can be filtered by a deviceID, a ruleId, an appId, a tagId or a combination of any (however, combining deviceId and ruleId will return an empty result set).");
    params.insert("o:deviceId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:ruleId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("o:appId", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("o:tagId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("GetTags", params);
    returns.insert("tagError", JsonTypes::tagErrorRef());
    returns.insert("o:tags", QVariantList() << JsonTypes::tagRef());
    setReturns("GetTags", returns);

    params.clear(); returns.clear();
    setDescription("AddTag", "Add a Tag. A Tag must have a deviceId OR a ruleId (call this method twice if you want to attach the same tag to a device and a rule), an appId (Use the appId of your app), a tagId (e.g. \"favorites\") and a value. Upon success, a TagAdded notification will be emitted. Calling this method twice for the same ids (device/rule, appId and tagId) but with a different value will update the tag's value and the TagValueChanged notification will be emitted.");
    params.insert("tag", JsonTypes::tagRef());
    setParams("AddTag", params);
    returns.insert("tagError", JsonTypes::tagErrorRef());
    setReturns("AddTag", returns);

    params.clear(); returns.clear();
    setDescription("RemoveTag", "Remove a Tag. Tag value is optional and will be disregarded. If the ids match, the tag will be deleted and a TagRemoved notification will be emitted.");
    params.insert("tag", JsonTypes::tagRef());
    setParams("RemoveTag", params);
    returns.insert("tagError", JsonTypes::tagErrorRef());
    setReturns("RemoveTag", returns);

    // Notifications
    params.clear();
    setDescription("TagAdded", "Emitted whenever a tag is added to the system. ");
    params.insert("tag", JsonTypes::tagRef());
    setParams("TagAdded", params);
    connect(NymeaCore::instance()->tagsStorage(), &TagsStorage::tagAdded, this, &TagsHandler::onTagAdded);

    params.clear();
    setDescription("TagRemoved", "Emitted whenever a tag is removed from the system. ");
    params.insert("tag", JsonTypes::tagRef());
    setParams("TagRemoved", params);
    connect(NymeaCore::instance()->tagsStorage(), &TagsStorage::tagRemoved, this, &TagsHandler::onTagRemoved);

    params.clear();
    setDescription("TagValueChanged", "Emitted whenever a tag's value is changed in the system. ");
    params.insert("tag", JsonTypes::tagRef());
    setParams("TagValueChanged", params);
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
        ret.append(JsonTypes::packTag(tag));
    }
    QVariantMap returns = statusToReply(TagsStorage::TagErrorNoError);
    returns.insert("tags", ret);
    return createReply(returns);

}

JsonReply *TagsHandler::AddTag(const QVariantMap &params) const
{
    Tag tag = JsonTypes::unpackTag(params.value("tag").toMap());
    TagsStorage::TagError error = NymeaCore::instance()->tagsStorage()->addTag(tag);
    QVariantMap returns = statusToReply(error);
    return createReply(returns);
}

JsonReply *TagsHandler::RemoveTag(const QVariantMap &params) const
{
    Tag tag = JsonTypes::unpackTag(params.value("tag").toMap());
    TagsStorage::TagError error = NymeaCore::instance()->tagsStorage()->removeTag(tag);
    QVariantMap returns = statusToReply(error);
    return createReply(returns);
}

void TagsHandler::onTagAdded(const Tag &tag)
{
    qCDebug(dcJsonRpc) << "Notify \"Tags.TagAdded\"";
    QVariantMap params;
    params.insert("tag", JsonTypes::packTag(tag));
    emit TagAdded(params);
}

void TagsHandler::onTagRemoved(const Tag &tag)
{
    qCDebug(dcJsonRpc) << "Notify \"Tags.TagRemoved\"";
    QVariantMap params;
    params.insert("tag", JsonTypes::packTag(tag));
    emit TagRemoved(params);
}

void TagsHandler::onTagValueChanged(const Tag &tag)
{
    qCDebug(dcJsonRpc) << "Notify \"Tags.TagValueChanged\"";
    QVariantMap params;
    params.insert("tag", JsonTypes::packTag(tag));
    emit TagValueChanged(params);
}

}
