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

#include "macaddressdatabase.h"
#include "loggingcategories.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QTimer>
#include <QSqlDatabase>
#include <QDir>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

NYMEA_LOGGING_CATEGORY(dcMacAddressDatabase, "MacAddressDatabase")

namespace nymeaserver {

MacAddressDatabase::MacAddressDatabase(QObject *parent) : QObject(parent)
{
    // Find database in system data locations
    QString databaseFileName;
    const QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    foreach (const QString &dataLocation, dataLocations) {
        const QStringList candidateFiles = {
            dataLocation + QDir::separator() + "nymea" + QDir::separator() + "nymead" + QDir::separator() + "mac-addresses.db",
            dataLocation + QDir::separator() + "nymea" + QDir::separator() + "mac-addresses.db",
            dataLocation + QDir::separator() + "mac-addresses.db"
        };

        foreach (const QString &candidate, candidateFiles) {
            QFileInfo databaseFileInfo(candidate);
            if (!databaseFileInfo.exists())
                continue;

            databaseFileName = databaseFileInfo.absoluteFilePath();
            break;
        }

        if (!databaseFileName.isEmpty())
            break;

    }

    if (databaseFileName.isEmpty()) {
        qCWarning(dcMacAddressDatabase()) << "Could not find the mac address database in any system data location paths" << dataLocations;
        qCWarning(dcMacAddressDatabase()) << "The mac address database lookup feature will not be available.";
        return;
    }


    m_databaseName = databaseFileName;

    m_available = initDatabase();
    if (m_available) {
        m_futureWatcher = new QFutureWatcher<QString>(this);
        connect(m_futureWatcher, &QFutureWatcher<QString>::finished, this, &MacAddressDatabase::onLookupFinished);
    }
}

MacAddressDatabase::MacAddressDatabase(const QString &databaseName, QObject *parent) :
    QObject(parent),
    m_databaseName(databaseName)
{
    m_available = initDatabase();
    if (m_available) {
        m_futureWatcher = new QFutureWatcher<QString>(this);
        connect(m_futureWatcher, &QFutureWatcher<QString>::finished, this, &MacAddressDatabase::onLookupFinished);
    }
}

MacAddressDatabase::~MacAddressDatabase()
{
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool MacAddressDatabase::available() const
{
    return m_available;
}

MacAddressDatabaseReply *MacAddressDatabase::lookupMacAddress(const QString &macAddress)
{
    MacAddressDatabaseReplyImpl *reply = new MacAddressDatabaseReplyImpl(this);
    connect(reply, &MacAddressDatabaseReply::finished, reply, &MacAddressDatabaseReply::deleteLater);
    reply->m_macAddress = macAddress;

    if (!m_available) {
        QTimer::singleShot(0, this, [=](){ emit reply->finished(); });
        return reply;
    }

    m_pendingReplies.enqueue(reply);
    runNextLookup();
    return reply;
}

bool MacAddressDatabase::initDatabase()
{
    qCDebug(dcMacAddressDatabase()) << "Starting to initialize the mac address database:" << m_databaseName;
    m_connectionName = QFileInfo(m_databaseName).baseName();
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(m_databaseName);

    if (!m_db.isValid()) {
        qCWarning(dcMacAddressDatabase()) << "The network database is not valid" << m_db.databaseName();
        return false;
    }

    m_db.close();
    if (!m_db.open()) {
        qCWarning(dcMacAddressDatabase()) << "Could not open database" << m_db.databaseName() << "Initialization failed.";
        return false;
    }

    // Verify the tables we need exist
    qCDebug(dcMacAddressDatabase()) << "Tables" << m_db.tables();
    if (!m_db.tables().contains("oui")) {
        qCWarning(dcMacAddressDatabase()) << "Invalid database. Could not find \"oui\" table in" << m_db.databaseName();
        return false;
    }

    if (!m_db.tables().contains("companyNames")) {
        qCWarning(dcMacAddressDatabase()) << "Invalid database. Could not find \"companyNames\" table in" << m_db.databaseName();
        return false;
    }

    qCInfo(dcMacAddressDatabase()) << "Database initialized successfully" << m_databaseName;
    return true;
}

void MacAddressDatabase::runNextLookup()
{
    if (m_pendingReplies.isEmpty())
        return;

    if (m_futureWatcher->isRunning() || m_currentReply)
        return;

    m_currentReply = m_pendingReplies.dequeue();
    m_currentReply->m_startTimestamp = QDateTime::currentMSecsSinceEpoch();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QFuture<QString> future = QtConcurrent::run(&MacAddressDatabase::lookupMacAddressVendorInternal, this, m_currentReply->macAddress());
#else
    QFuture<QString> future = QtConcurrent::run(this, &MacAddressDatabase::lookupMacAddressVendorInternal, m_currentReply->macAddress());
#endif
    m_futureWatcher->setFuture(future);
}

void MacAddressDatabase::onLookupFinished()
{
    if (m_currentReply) {
        QString manufacturer = m_futureWatcher->future().result();
        qCInfo(dcMacAddressDatabase()) << "Manufacturer lookup for" << m_currentReply->macAddress() << "finished:" << manufacturer << QDateTime::currentMSecsSinceEpoch() - m_currentReply->m_startTimestamp << "ms";
        m_currentReply->m_manufacturer = manufacturer;
        emit m_currentReply->finished();
        m_currentReply = nullptr;
    }

    runNextLookup();
}

QString MacAddressDatabase::lookupMacAddressVendorInternal(const QString &macAddress)
{
    qCInfo(dcMacAddressDatabase()) << "Start looking up vendor for" << macAddress;
    // Convert the mac address string to upper like in the database and remove : since they have been removed for size reasons
    QString fullMacAddressString = QString(macAddress).toUpper().remove(":");

    QString manufacturer;
    int length = 6;
    while (true) {
        QString searchString = fullMacAddressString.left(length);
        QString queryString = QString("SELECT COUNT(oui) FROM oui WHERE oui LIKE \'%1%\';").arg(searchString);
        qCDebug(dcMacAddressDatabase()) << "Query:" << queryString;
        QSqlQuery countQuery(m_db);
        if (!countQuery.exec(queryString)) {
            qCWarning(dcMacAddressDatabase()) << "Unable to execute SQL query" << queryString << m_db.lastError().databaseText() << m_db.lastError().driverText();
            break;
        }

        if (countQuery.lastError().isValid()) {
            qCWarning(dcMacAddressDatabase()) << "Query finished with error" << countQuery.lastError().text();
            break;
        }

        if (!countQuery.next())
            break;

        int rowCount = countQuery.value(0).toInt();
        qCDebug(dcMacAddressDatabase()) << "Found" << rowCount << "with" << searchString;
        // If we have found the one...
        if (rowCount == 1) {
            // Query the name
            queryString = QString("SELECT companyName from companyNames WHERE rowid IS (SELECT companyNameIndex FROM oui WHERE oui=\'%1\');").arg(searchString);
            qCDebug(dcMacAddressDatabase()) << "Query:" << queryString;
            QSqlQuery rowQuery(m_db);
            if (!rowQuery.exec(queryString)) {
                qCWarning(dcMacAddressDatabase()) << "Unable to execute SQL query" << queryString << m_db.lastError().databaseText() << m_db.lastError().driverText();
                break;
            }

            if (!rowQuery.next())
                break;

            manufacturer = rowQuery.value(0).toString();
            break;
        }

        // If nothing found
        if (rowCount == 0)
            break;

        // Found to many results, lets add a value until we find the matching vendor
        length += 1;
        if (length > fullMacAddressString.length())
            break;

        // Search with one addition digit
    }

    return manufacturer;
}

}
