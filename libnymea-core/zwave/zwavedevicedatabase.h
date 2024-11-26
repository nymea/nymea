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

}

#endif // ZWAVEDEVICEDATABASE_H
