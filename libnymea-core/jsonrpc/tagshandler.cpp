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

#include "tagshandler.h"

#include "nymeacore.h"
#include "tagging/tagsstorage.h"

namespace nymeaserver {

TagsHandler::TagsHandler(QObject *parent) : JsonHandler(parent)
{
    // Enums
    registerEnum<TagsStorage::TagError>();

    // Objects
    registerObject<Tag, Tags>();

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the Tags matching the given filter. "
                  "Tags can be filtered by a thingID, a ruleId, an appId, a tagId or a combination of any (however, "
                  "combining thingId and ruleId will return an empty result set).";
    params.insert("o:thingId", enumValueName(Uuid));
    params.insert("o:ruleId", enumValueName(Uuid));
    params.insert("o:appId", enumValueName(String));
    params.insert("o:tagId", enumValueName(String));
    returns.insert("tagError", enumRef<TagsStorage::TagError>());
    returns.insert("o:tags", objectRef("Tags"));
    registerMethod("GetTags", description, params, returns, Types::PermissionScopeControlThings);

    params.clear(); returns.clear();
    description = "Add a Tag. "
                  "A Tag must have a thingId OR a ruleId (call this method twice if you want to attach the same tag "
                  "to a thing and a rule), an appId (Use the appId of your app), a tagId (e.g. \"favorites\") and a "
                  "value. Upon success, a TagAdded notification will be emitted. Calling this method twice for the "
                  "same ids (thing/rule, appId and tagId) but with a different value will update the tag's value and "
                  "the TagValueChanged notification will be emitted.";
    params.insert("tag", objectRef("Tag"));
    returns.insert("tagError", enumRef<TagsStorage::TagError>());
    registerMethod("AddTag", description, params, returns, Types::PermissionScopeControlThings);

    params.clear(); returns.clear();
    description = "Remove a Tag. "
                  "Tag value is optional and will be disregarded. If the ids match, the tag will be deleted and a "
                  "TagRemoved notification will be emitted.";
    params.insert("tag", objectRef("Tag"));
    returns.insert("tagError", enumRef<TagsStorage::TagError>());
    registerMethod("RemoveTag", description, params, returns, Types::PermissionScopeControlThings);

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
        if (params.contains("thingId") && params.value("thingId").toUuid() != tag.thingId()) {
            continue;
        }
        if (params.contains("ruleId") && params.value("ruleId").toUuid() != tag.ruleId()) {
            continue;
        }
        if (params.contains("appId") && params.value("appId").toString() != tag.appId()) {
            continue;
        }
        if (params.contains("tagId") && params.value("tagId").toString() != tag.tagId()) {
            continue;
        }
        ret.append(pack(tag));
    }
    QVariantMap returns = statusToReply(TagsStorage::TagErrorNoError);
    returns.insert("tags", ret);
    return createReply(returns);

}

JsonReply *TagsHandler::AddTag(const QVariantMap &params) const
{
    Tag tag = unpack<Tag>(params.value("tag").toMap());
    TagsStorage::TagError error = NymeaCore::instance()->tagsStorage()->addTag(tag);
    QVariantMap returns = statusToReply(error);
    return createReply(returns);
}

JsonReply *TagsHandler::RemoveTag(const QVariantMap &params) const
{
    Tag tag = unpack<Tag>(params.value("tag").toMap());
    TagsStorage::TagError error = NymeaCore::instance()->tagsStorage()->removeTag(tag);
    QVariantMap returns = statusToReply(error);
    return createReply(returns);
}

void TagsHandler::onTagAdded(const Tag &tag)
{
    qCDebug(dcJsonRpc()) << "Notify \"Tags.TagAdded\"";
    QVariantMap params;
    params.insert("tag", pack(tag));
    emit TagAdded(params);
}

void TagsHandler::onTagRemoved(const Tag &tag)
{
    qCDebug(dcJsonRpc()) << "Notify \"Tags.TagRemoved\"";
    QVariantMap params;
    params.insert("tag", pack(tag));
    emit TagRemoved(params);
}

void TagsHandler::onTagValueChanged(const Tag &tag)
{
    qCDebug(dcJsonRpc()) << "Notify \"Tags.TagValueChanged\"";
    QVariantMap params;
    params.insert("tag", pack(tag));
    emit TagValueChanged(params);
}

QVariantMap TagsHandler::statusToReply(TagsStorage::TagError status) const
{
    QVariantMap returns;
    returns.insert("tagError", enumValueName<TagsStorage::TagError>(status));
    return returns;
}

}
