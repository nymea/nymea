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

#ifndef PARAM_H
#define PARAM_H

#include "libnymea.h"
#include "typeutils.h"

#include <QString>
#include <QVariant>

class LIBNYMEA_EXPORT Param
{
    Q_GADGET
    Q_PROPERTY(QUuid paramTypeId READ paramTypeId WRITE setParamTypeId USER true)
    Q_PROPERTY(QVariant value READ value WRITE setValue)
public:
    Param(const ParamTypeId &paramTypeId = ParamTypeId(), const QVariant &value = QVariant());

    ParamTypeId paramTypeId() const;
    void setParamTypeId(const ParamTypeId &paramTypeId);

    QVariant value() const;
    void setValue(const QVariant &value);

    bool isValid() const;

private:
    ParamTypeId m_paramTypeId;
    QVariant m_value;
};

Q_DECLARE_METATYPE(Param)
QDebug operator<<(QDebug dbg, const Param &param);

class LIBNYMEA_EXPORT ParamList: public QList<Param>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ParamList();
    ParamList(const QList<Param> &other);
    ParamList(std::initializer_list<Param> args):QList(args) {}
    Q_INVOKABLE QVariant get(int index);
    Q_INVOKABLE void put(const QVariant &variant);
    bool hasParam(const ParamTypeId &paramTypeId) const;
    QVariant paramValue(const ParamTypeId &paramTypeId) const;
    bool setParamValue(const ParamTypeId &paramTypeId, const QVariant &value);
    ParamList operator<<(const Param &param);

private:
    QList<ParamTypeId> m_ids;

};
Q_DECLARE_METATYPE(ParamList)
QDebug operator<<(QDebug dbg, const ParamList &params);

#endif // PARAM_H
