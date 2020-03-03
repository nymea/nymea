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
