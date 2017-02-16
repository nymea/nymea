/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PARAM_H
#define PARAM_H

#include "libguh.h"
#include "typeutils.h"

#include <QString>
#include <QVariant>

class LIBGUH_EXPORT Param
{
public:
    Param(const ParamTypeId &paramTypeId = ParamTypeId(), const QVariant &value = QVariant());

    ParamTypeId paramTypeId() const;

    QVariant value() const;
    void setValue(const QVariant &value);

    bool isValid() const;

private:
    ParamTypeId m_paramTypeId;
    QVariant m_value;
};

Q_DECLARE_METATYPE(Param)
QDebug operator<<(QDebug dbg, const Param &param);

class LIBGUH_EXPORT ParamList: public QList<Param>
{
public:
    bool hasParam(const ParamTypeId &paramTypeId) const;
    QVariant paramValue(const ParamTypeId &paramTypeId) const;
    bool setParamValue(const ParamTypeId &paramTypeId, const QVariant &value);
    ParamList operator<<(const Param &param);

private:
    QList<ParamTypeId> m_ids;

};

QDebug operator<<(QDebug dbg, const ParamList &params);

#endif // PARAM_H
