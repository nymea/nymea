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

#ifndef TAGSHANDLER_H
#define TAGSHANDLER_H

#include <QObject>

#include "jsonhandler.h"

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
};

}

#endif // TAGSHANDLER_H
