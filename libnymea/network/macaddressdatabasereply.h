// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MACADDRESSDATABASEREPLY_H
#define MACADDRESSDATABASEREPLY_H

#include <QObject>

#include "libnymea.h"

class LIBNYMEA_EXPORT MacAddressDatabaseReply : public QObject
{
    Q_OBJECT

public:
    explicit MacAddressDatabaseReply(QObject *parent = nullptr);
    virtual ~MacAddressDatabaseReply() = default;

    virtual QString macAddress() const = 0;
    virtual QString manufacturer() const  = 0;

signals:
    void finished();

};

#endif // MACADDRESSDATABASEREPLY_H
