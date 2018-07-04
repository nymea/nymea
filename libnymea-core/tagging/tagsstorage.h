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

#ifndef TAGSSTORAGE_H
#define TAGSSTORAGE_H

#include "tag.h"
#include "typeutils.h"

#include <QObject>
#include <QVector>

class DeviceManager;

namespace nymeaserver {

class RuleEngine;

class TagsStorage : public QObject
{
    Q_OBJECT
public:
    enum TagError {
        TagErrorNoError,
        TagErrorDeviceNotFound,
        TagErrorRuleNotFound,
        TagErrorTagNotFound
    };
    Q_ENUM(TagError)

    explicit TagsStorage(DeviceManager* deviceManager, RuleEngine* ruleEngine, QObject *parent = nullptr);

    TagError addTag(const Tag &tag);
    TagError removeTag(const Tag &tag);

    QList<Tag> tags() const;
    QList<Tag> tags(const DeviceId &deviceId) const;
    QList<Tag> tags(const RuleId &ruleId) const;

signals:
    void tagAdded(const Tag &tag);
    void tagRemoved(const Tag &tag);
    void tagValueChanged(const Tag &tag);

private slots:
    void deviceRemoved(const DeviceId &deviceId);
    void ruleRemoved(const RuleId &ruleId);

private:
    void saveTag(const Tag &tag);
    void unsaveTag(const Tag &tag);

private:
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;
    QList<Tag> m_tags;
};

}

#endif // TAGSSTORAGE_H
