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

#ifndef ZWAVEDEVICEDATABASE_H
#define ZWAVEDEVICEDATABASE_H

#include <QSqlDatabase>

#include "hardware/zwave/zwavenode.h"

namespace nymeaserver {

class ZWaveManager;

class ZWaveDeviceDatabase
{
public:
    explicit ZWaveDeviceDatabase(const QString &path, const QUuid &networkUuid);

    bool initDB();
    void removeDB();
    void clearDB();

    void storeNode(ZWaveNode *node);
    void removeNode(quint8 nodeId);
    void storeValue(ZWaveNode *node, quint64 valueId);
    void removeValue(ZWaveNode *node, quint64 valueId);
    ZWaveNodes createNodes(ZWaveManager *manager);

private:
    QString m_path;
    QUuid m_networkUuid;
    QSqlDatabase m_db;
};

} // namespace nymeaserver

#endif // ZWAVEDEVICEDATABASE_H
