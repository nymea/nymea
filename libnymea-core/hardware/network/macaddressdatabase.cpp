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

#include "macaddressdatabase.h"
#include "loggingcategories.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QTimer>
#include <QSqlDatabase>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

NYMEA_LOGGING_CATEGORY(dcMacAddressDatabase, "MacAddressDatabase")

namespace nymeaserver {

MacAddressDatabase::MacAddressDatabase(QObject *parent) : QObject(parent)
{
    // Find database in system data locations
    QString databaseFileName;
    foreach (const QString &dataLocation, QStandardPaths::standardLocations(QStandardPaths::DataLocation)) {
        QFileInfo databaseFileInfo(dataLocation + QDir::separator() + "mac-addresses.db");
        if (!databaseFileInfo.exists()) {
            continue;
        }

        databaseFileName = databaseFileInfo.absoluteFilePath();
        break;
    }

    if (databaseFileName.isEmpty()) {
        qCWarning(dcMacAddressDatabase()) << "Could not find the mac address database in any system data location paths" << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
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
    QFuture<QString> future = QtConcurrent::run(this, &MacAddressDatabase::lookupMacAddressVendorInternal, m_currentReply->macAddress());
    m_futureWatcher->setFuture(future);
}

void MacAddressDatabase::onLookupFinished()
{
    if (m_currentReply) {
        QString manufacturer = m_futureWatcher->future().result();
        qCDebug(dcMacAddressDatabase()) << "Manufacturer lookup for" << m_currentReply->macAddress() << "finished:" << manufacturer << QDateTime::currentMSecsSinceEpoch() - m_currentReply->m_startTimestamp << "ms";
        m_currentReply->m_manufacturer = manufacturer;
        emit m_currentReply->finished();
        m_currentReply = nullptr;
    }

    runNextLookup();
}

QString MacAddressDatabase::lookupMacAddressVendorInternal(const QString &macAddress)
{
    qCDebug(dcMacAddressDatabase()) << "Start looking up vendor for" << macAddress;
    // Convert the mac address string to upper like in the database and remove : since they have been removed for size reasons
    QString fullMacAddressString = QString(macAddress).toUpper().remove(":");

    QString manufacturer;
    int length = 6;
    while (true) {
        QString searchString = fullMacAddressString.left(length);
        QString queryString = QString("SELECT COUNT(oui) FROM oui WHERE oui LIKE \'%1%\';").arg(searchString);
        qCDebug(dcMacAddressDatabase()) << "Query:" << queryString;
        QSqlQuery countQuery = m_db.exec(queryString);
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
            countQuery = m_db.exec(queryString);
            if (!countQuery.next())
                break;

            manufacturer = countQuery.value(0).toString();
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
