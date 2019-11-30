/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef ACTIONHANDLER_H
#define ACTIONHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "devices/devicemanager.h"

namespace nymeaserver {

class ActionHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ActionHandler(QObject *parent = nullptr);

    QString name() const;

    Q_INVOKABLE JsonReply *ExecuteAction(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetActionType(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *ExecuteBrowserItem(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ExecuteBrowserItemAction(const QVariantMap &params);

};

}

#endif // ACTIONHANDLER_H
