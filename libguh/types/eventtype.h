/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef TRIGGERTYPE_H
#define TRIGGERTYPE_H

#include "libguh.h"
#include "typeutils.h"
#include "paramtype.h"

#include <QVariantMap>

class LIBGUH_EXPORT EventType
{
public:
    EventType(const EventTypeId &id);

    EventTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    int index() const;
    void setIndex(const int &index);

    QList<ParamType> paramTypes() const;
    void setParamTypes(const QList<ParamType> &paramTypes);

    bool ruleRelevant() const;
    void setRuleRelevant(const bool &ruleRelevant);

    bool graphRelevant() const;
    void setGraphRelevant(const bool &graphRelevant);

private:
    EventTypeId m_id;
    QString m_name;
    int m_index;
    QList<ParamType> m_paramTypes;
    bool m_ruleRelevant;
    bool m_graphRelevant;
};

#endif // TRIGGERTYPE_H
