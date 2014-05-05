/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ACTIONHANDLER_H
#define ACTIONHANDLER_H

#include "jsonhandler.h"
#include "devicemanager.h"

class ActionHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit ActionHandler(QObject *parent = 0);

    QString name() const;

    Q_INVOKABLE JsonReply* ExecuteAction(const QVariantMap &params);

    Q_INVOKABLE JsonReply* GetActionType(const QVariantMap &params) const;

private slots:
    void actionExecuted(const ActionId &id, DeviceManager::DeviceError status, const QString &errorMessage);

private:
    QVariantMap statusToReply(DeviceManager::DeviceError status, const QString &errorMessage);

private:
    QHash<ActionId, JsonReply*> m_asyncActionExecutions;
};

#endif // ACTIONHANDLER_H
