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

#ifndef TAGSHANDLER_H
#define TAGSHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"
#include "tagging/tagsstorage.h"

namespace nymeaserver {

class TagsHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit TagsHandler(QObject *parent = nullptr);
    QString name() const override;

    Q_INVOKABLE JsonReply *GetTags(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *AddTag(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *RemoveTag(const QVariantMap &params) const;

signals:
    void TagAdded(const QVariantMap &params);
    void TagRemoved(const QVariantMap &params);
    void TagValueChanged(const QVariantMap &params);

private slots:
    void onTagAdded(const Tag &tag);
    void onTagRemoved(const Tag &tag);
    void onTagValueChanged(const Tag &tag);

private:
    QVariantMap statusToReply(TagsStorage::TagError status) const;
};

} // namespace nymeaserver

#endif // TAGSHANDLER_H
