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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TRIGGERTYPE_H
#define TRIGGERTYPE_H

#include "libnymea.h"
#include "typeutils.h"
#include "paramtype.h"

#include <QVariantMap>

class LIBNYMEA_EXPORT EventType
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)
    Q_PROPERTY(int index READ index)
    Q_PROPERTY(ParamTypes paramTypes READ paramTypes WRITE setParamTypes)

public:
    EventType();
    EventType(const EventTypeId &id);

    EventTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int index() const;
    void setIndex(const int &index);

    ParamTypes paramTypes() const;
    void setParamTypes(const ParamTypes &paramTypes);

    bool suggestLogging() const;
    void setSuggestLogging(bool logged);

    bool isValid() const;

private:
    EventTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index;
    QList<ParamType> m_paramTypes;
    bool m_logged = false;
};
Q_DECLARE_METATYPE(EventType)

class EventTypes: public QList<EventType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    EventTypes() = default;
    EventTypes(const QList<EventType> &other);
    bool contains(const EventTypeId &id) const;
    bool contains(const QString &name) const;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    EventType findByName(const QString &name);
    EventType findById(const EventTypeId &id);
    EventType &operator[](const QString &name);
};
Q_DECLARE_METATYPE(EventTypes)

#endif // TRIGGERTYPE_H
