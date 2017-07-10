/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ACTIONTYPE_H
#define ACTIONTYPE_H

#include "libguh.h"
#include "typeutils.h"
#include "paramtype.h"

#include <QVariantList>

class LIBGUH_EXPORT ActionType
{
public:
    ActionType(const ActionTypeId &id);

    ActionTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    int index() const;
    void setIndex(const int &index);

    QList<ParamType> paramTypes() const;
    void setParamTypes(const QList<ParamType> &paramTypes);

private:
    ActionTypeId m_id;
    QString m_name;
    int m_index;
    QList<ParamType> m_paramTypes;
};

class ActionTypes: public QList<ActionType>
{
public:
    ActionTypes(const QList<ActionType> &other);
    ActionType findByName(const QString &name);
    ActionType findById(const ActionTypeId &id);
};

#endif // ACTIONTYPE_H
