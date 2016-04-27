/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef KODIREPLY_H
#define KODIREPLY_H

#include <QVariant>
#include <QString>

#include "typeutils.h"

class KodiReply
{
public:
    KodiReply();
    KodiReply(const QString &method, const QVariantMap &params, const ActionId &actionId = ActionId());

    void setActionId(const ActionId &actionId);
    ActionId actionId() const;

    void setMethod(const QString &method);
    QString method() const;

    void setParams(const QVariantMap &params);
    QVariantMap params() const;

private:
    QString m_method;
    QVariantMap m_params;
    ActionId m_actionId;

};

#endif // KODIREPLY_H
