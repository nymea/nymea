/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
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

#ifndef BROWSERITEMACTION_H
#define BROWSERITEMACTION_H

#include "typeutils.h"

class BrowserItemAction
{
public:
    BrowserItemAction();

    explicit BrowserItemAction(const DeviceId &deviceId = DeviceId(), const QString &itemId = QString());
    BrowserItemAction(const BrowserItemAction &other);

    ActionId id() const;

    bool isValid() const;

    DeviceId deviceId() const;
    QString itemId() const;

    void operator=(const BrowserItemAction &other);
private:
    ActionId m_id;
    DeviceId m_deviceId;
    QString m_itemId;
};

#endif // BROWSERITEMACTION_H
