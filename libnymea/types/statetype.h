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

#ifndef STATETYPE_H
#define STATETYPE_H

#include "libnymea.h"
#include "typeutils.h"

#include <QVariant>

class LIBNYMEA_EXPORT StateType
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)
    Q_PROPERTY(QVariant::Type type READ type WRITE setType)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue)
    Q_PROPERTY(Types::Unit unit READ unit WRITE setUnit USER true)
    Q_PROPERTY(Types::IOType ioType READ ioType WRITE setIOType USER true)
    Q_PROPERTY(QVariant minValue READ minValue WRITE setMinValue USER true)
    Q_PROPERTY(QVariant maxValue READ maxValue WRITE setMaxValue USER true)
    Q_PROPERTY(QVariantList possibleValues READ possibleValues WRITE setPossibleValues USER true)

public:
    StateType();
    StateType(const StateTypeId &id);

    StateTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int index() const;
    void setIndex(const int &index);

    QVariant::Type type() const;
    void setType(const QVariant::Type &type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

    QVariantList possibleValues() const;
    void setPossibleValues(const QVariantList &possibleValues);

    Types::Unit unit() const;
    void setUnit(const Types::Unit &unit);

    Types::IOType ioType() const;
    void setIOType(Types::IOType ioType);

    bool writable() const;
    void setWritable(bool writable);

    bool cached() const;
    void setCached(bool cached);

    static QStringList typeProperties();
    static QStringList mandatoryTypeProperties();

    bool isValid() const;

private:
    StateTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index = 0;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    QVariantList m_possibleValues;
    Types::Unit m_unit = Types::UnitNone;
    Types::IOType m_ioType = Types::IOTypeNone;
    bool m_writable = false;
    bool m_cached = true;
};
Q_DECLARE_METATYPE(StateType)

class StateTypes: public QList<StateType>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    StateTypes() = default;
    StateTypes(const QList<StateType> &other);
    bool contains(const StateTypeId &stateTypeId);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
    StateType findByName(const QString &name);
    StateType findById(const StateTypeId &id);
};
Q_DECLARE_METATYPE(StateTypes)

#endif // STATETYPE_H
