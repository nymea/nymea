/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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
