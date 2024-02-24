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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zwavedevicedatabase.h"
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlResult>
#include <QRegularExpression>
#include <QDataStream>

#include "hardware/zwave/zwavenode.h"
#include "zwavenodeimplementation.h"

#include "loggingcategories.h"
Q_DECLARE_LOGGING_CATEGORY(dcZWave)

namespace nymeaserver {

ZWaveDeviceDatabase::ZWaveDeviceDatabase(const QString &path, const QUuid &networkUuid):
    m_path(path),
    m_networkUuid(networkUuid)
{
}

bool ZWaveDeviceDatabase::initDB()
{
    m_db.close();

    QDir path(m_path);
    if (!path.exists()) {
        if (!path.mkpath(path.path())) {
            qCCritical(dcZWave) << "Unable to create ZWave divce database path";
            return false;
        }
    }

    QString networkUuidString = m_networkUuid.toString().remove(QRegularExpression("[{}]"));
    m_db = QSqlDatabase::addDatabase("QSQLITE", "ZWaveDevices-" + networkUuidString);
    m_db.setDatabaseName(path.absoluteFilePath("zwave-network-" + networkUuidString + ".db"));

    bool opened = m_db.open();
    if (!opened) {
        qCCritical(dcZWave()) << "Cannot open ZWave device DB at" << m_db.databaseName() << m_db.lastError().databaseText();
        return false;
    }

    if (!m_db.tables().contains("metadata")) {
        qCDebug(dcZWave()) << "No \"metadata\" table in database. Creating it.";
        QSqlQuery query = QSqlQuery("CREATE TABLE metadata (version INT);", m_db);
        if (!query.exec()) {
            qCWarning(dcUserManager()) << "Unable to execute SQL query" << query.executedQuery() << m_db.lastError().databaseText() << m_db.lastError().driverText();
            return false;
        }

        query = QSqlQuery("INSERT INTO metadata (version) VALUES (1);", m_db);
        if (!query.exec() || m_db.lastError().isValid()) {
            qCCritical(dcZWave()) << "Error creating metadata table in devie database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }
    }

    if (!m_db.tables().contains("nodes")) {
        qCDebug(dcZWave()) << "No \"nodes\" table in database. Creating it.";

        QSqlQuery query = QSqlQuery("CREATE TABLE nodes "
                                    "("
                                    "nodeId INT PRIMARY KEY NOT NULL,"
                                    "basicType INT,"
                                    "deviceType INT,"
                                    "plusDeviceType INT,"
                                    "manufacturerId INT,"
                                    "manufacturerName TEXT,"
                                    "name TEXT,"
                                    "productId INT,"
                                    "productName TEXT,"
                                    "productType INT,"
                                    "isZWavePlus INT,"
                                    "isSecure INT,"
                                    "isBeaming INT,"
                                    "version INT"
                                    ");", m_db);


        if (!query.exec() || m_db.lastError().isValid()) {
            qCCritical(dcZWave()) << "Error creating nodes table in devices database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }
    }

    if (!m_db.tables().contains("nodevalues")) {
        qCDebug(dcZWave()) << "No \"nodevalues\" table in database. Creating it.";
        QSqlQuery query = QSqlQuery("CREATE TABLE nodevalues "
                  "("
                  "valueId INT PRIMARY KEY NOT NULL,"
                  "nodeId INT,"
                  "valueGenre INT,"
                  "commandClass INT,"
                  "instance INT,"
                  "idx INT,"
                  "type INT,"
                  "value TEXT,"
                  "valueSelection INT,"
                  "description TEXT,"
                  "FOREIGN KEY (nodeId) REFERENCES nodes(nodeId)"
                  ");", m_db);

        if (!query.exec() || m_db.lastError().isValid()) {
            qCCritical(dcZWave()) << "Error creating nodevalues table in device database. Driver error:" << m_db.lastError().driverText() << "Database error:" << m_db.lastError().databaseText();
            return false;
        }
    }

    qCInfo(dcZWave()) << "Initialized devices DB successfully." << m_db.databaseName();
    return true;
}

void ZWaveDeviceDatabase::removeDB()
{
    m_db.close();
    QFile::remove(m_db.databaseName());
}

void ZWaveDeviceDatabase::clearDB()
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM nodes;");
    if (!query.exec()) {
        qCWarning(dcZWave) << "Error clearing node db:" << query.lastError().databaseText() << query.lastError().driverText();
        qCDebug(dcZWave) << "Query was:" << query.executedQuery();
        return;
    }
}

void ZWaveDeviceDatabase::storeNode(ZWaveNode *node)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO nodes(nodeId, basicType, deviceType, plusDeviceType, manufacturerId, manufacturerName, name, productId, productName, productType, isZWavePlus, isSecure, isBeaming, version) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    query.addBindValue(node->nodeId());
    query.addBindValue(node->nodeType());
    query.addBindValue(node->deviceType());
    query.addBindValue(node->plusDeviceType());
    query.addBindValue(node->manufacturerId());
    query.addBindValue(node->manufacturerName());
    query.addBindValue(node->name());
    query.addBindValue(node->productId());
    query.addBindValue(node->productName());
    query.addBindValue(node->productType());
    query.addBindValue(node->isZWavePlusDevice());
    query.addBindValue(node->isSecurityDevice());
    query.addBindValue(node->isBeamingDevice());
    query.addBindValue(node->version());
    if (!query.exec()) {
        qCWarning(dcZWave) << "Error inserting node into db:" << query.lastError().databaseText() << query.lastError().driverText();
        qCDebug(dcZWave) << "Query was:" << query.executedQuery();
        return;
    }

    foreach (const ZWaveValue &value, node->values()) {
        storeValue(node, value.id());
    }
}

void ZWaveDeviceDatabase::removeNode(quint8 nodeId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM nodes WHERE nodeId = ?;");
    query.addBindValue(nodeId);
    if (!query.exec()) {
        qCWarning(dcZWave) << "Error removing node from db:" << query.lastError().databaseText() << query.lastError().driverText();
        qCDebug(dcZWave) << "Query was:" << query.executedQuery();
        return;
    }
}

void ZWaveDeviceDatabase::storeValue(ZWaveNode *node, quint64 valueId)
{
    ZWaveValue value = node->value(valueId);

    QByteArray byteArray;
    QDataStream out(&byteArray, QIODevice::WriteOnly);
    out << value.value();

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO nodevalues(valueId, nodeId, valueGenre, commandClass, instance, idx, type, value, valueSelection, description) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    query.addBindValue(value.id());
    query.addBindValue(node->nodeId());
    query.addBindValue(value.genre());
    query.addBindValue(value.commandClass());
    query.addBindValue(value.instance());
    query.addBindValue(value.index());
    query.addBindValue(value.type());
    query.addBindValue(byteArray.toBase64());
    query.addBindValue(value.valueListSelection());
    query.addBindValue(value.description());
    if (!query.exec()) {
        qCWarning(dcZWave) << "Unable to store node value:" << query.lastError().databaseText();
    }
}

void ZWaveDeviceDatabase::removeValue(ZWaveNode *node, quint64 valueId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM nodevalues WHERE nodeId = ? AND valueId = ?;");
    query.addBindValue(node->nodeId());
    query.addBindValue(valueId);
    if (!query.exec()) {
        qCWarning(dcZWave) << "Unable to remove node value from DB:" << query.lastError().databaseText();
    }
}

ZWaveNodes ZWaveDeviceDatabase::createNodes(ZWaveManager *manager)
{
    ZWaveNodes ret;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM nodes;");
    if (!query.exec()) {
        qCWarning(dcZWave()) << "Unable to query nodes from DB" << query.lastError().databaseText();
        return ret;
    }


    while (query.next()) {
        quint8 nodeId = query.value("nodeId").toUInt();
        ZWaveNodeImplementation *node = new ZWaveNodeImplementation(manager, m_networkUuid, nodeId);
        node->setNodeType(static_cast<ZWaveNode::ZWaveNodeType>(query.value("basicType").toInt()));
        node->setDeviceType(static_cast<ZWaveNode::ZWaveDeviceType>(query.value("deviceType").toInt()));
        node->setPlusDeviceType(static_cast<ZWaveNode::ZWavePlusDeviceType>(query.value("plusDeviceType").toInt()));
        node->setManufacturerId(query.value("manufacturerId").toUInt());
        node->setManufacturerName(query.value("manufacturerName").toString());
        node->setName(query.value("name").toString());
        node->setProductId(query.value("productId").toUInt());
        node->setProductName(query.value("productName").toString());
        node->setProductType(query.value("productType").toUInt());
        node->setIsZWavePlusDevice(query.value("isZWavePlus").toBool());
        node->setIsSecurityDevice(query.value("isSecure").toBool());
        node->setIsBeamingDevice(query.value("isBeaming").toBool());
        node->setVersion(query.value("version").toUInt());

        QSqlQuery valueQuery(m_db);
        valueQuery.prepare("SELECT * FROM nodevalues WHERE nodeId = ?;");
        valueQuery.addBindValue(nodeId);
        if (!valueQuery.exec()) {
            qCWarning(dcZWave) << "Unable to query node values from DB" << query.lastError().databaseText();
            continue;
        }
        while (valueQuery.next()) {
            ZWaveValue value(
                        valueQuery.value("valueId").toULongLong(),
                        static_cast<ZWaveValue::Genre>(valueQuery.value("valueGenre").toInt()),
                        static_cast<ZWaveValue::CommandClass>(valueQuery.value("commandClass").toUInt()),
                        valueQuery.value("instance").toUInt(),
                        valueQuery.value("idx").toUInt(),
                        static_cast<ZWaveValue::Type>(valueQuery.value("type").toInt()),
                        valueQuery.value("description").toString());
            QByteArray data = QByteArray::fromBase64(valueQuery.value("value").toString().toUtf8());
            QDataStream inputStream(data);
            QVariant deseriealizedValue;
            inputStream >> deseriealizedValue;
            value.setValue(deseriealizedValue, valueQuery.value("valueSelection").toInt());
            node->updateValue(value);
        }

        ret.append(node);
    }

    return ret;
}

}
