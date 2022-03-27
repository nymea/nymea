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

#ifndef TAG_H
#define TAG_H

#include "typeutils.h"

#include <QString>
#include <QVariant>

namespace nymeaserver {

class Tag
{
    Q_GADGET
    Q_PROPERTY(QString appId READ appId WRITE setAppId)
    Q_PROPERTY(QString tagId READ tagId WRITE setTagId)
    Q_PROPERTY(QUuid thingId READ thingId WRITE setThingId USER true)
    Q_PROPERTY(QUuid ruleId READ ruleId WRITE setRuleId USER true)
    Q_PROPERTY(QString value READ value WRITE setValue USER true)
public:
    Tag();
    Tag(const ThingId &thingId, const QString &appId, const QString &tagId, const QString &value);
    Tag(const RuleId &ruleId, const QString &appId, const QString &tagId, const QString &value);

    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    RuleId ruleId() const;
    void setRuleId(const RuleId &ruleId);

    QString appId() const;
    void setAppId(const QString &appId);

    QString tagId() const;
    void setTagId(const QString &tagId);

    QString value() const;
    void setValue(const QString &value);

    bool operator==(const Tag &other) const;

private:
    ThingId m_thingId;
    RuleId m_ruleId;
    QString m_appId;
    QString m_tagId;
    QString m_value;
};

class Tags: public QList<Tag>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Tags();
    Tags(const QList<Tag> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

QDebug operator<<(QDebug dbg, const Tag &tag);
}

Q_DECLARE_METATYPE(nymeaserver::Tag)
Q_DECLARE_METATYPE(nymeaserver::Tags)

#endif // TAG_H
