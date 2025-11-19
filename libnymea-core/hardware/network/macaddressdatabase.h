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

#ifndef MACADDRESSDATABASE_H
#define MACADDRESSDATABASE_H

#include <QQueue>
#include <QObject>
#include <QSqlDatabase>
#include <QFutureWatcher>

#include "macaddressdatabasereplyimpl.h"

namespace nymeaserver {

class MacAddressDatabase : public QObject
{
    Q_OBJECT
public:
    explicit MacAddressDatabase(QObject *parent = nullptr);
    MacAddressDatabase(const QString &databaseName, QObject *parent = nullptr);
    ~MacAddressDatabase();

    bool available() const;

    MacAddressDatabaseReply *lookupMacAddress(const QString &macAddress);

private:
    QSqlDatabase m_db;
    bool m_available = false;
    QString m_connectionName;
    QString m_databaseName = "/usr/share/nymea/mac-addresses.db";

    MacAddressDatabaseReplyImpl *m_currentReply = nullptr;
    QFutureWatcher<QString> *m_futureWatcher = nullptr;
    QQueue<MacAddressDatabaseReplyImpl *> m_pendingReplies;

    bool initDatabase();
    void runNextLookup();

private slots:
    void onLookupFinished();
    QString lookupMacAddressVendorInternal(const QString &macAddress);

};

}

#endif // MACADDRESSDATABASE_H
