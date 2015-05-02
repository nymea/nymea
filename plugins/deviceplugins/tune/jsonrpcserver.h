/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QObject>
#include <QDebug>
#include <QJsonDocument>

#include "tunemanager.h"
#include "plugin/device.h"
#include "types/action.h"

class JsonRpcServer : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcServer(QObject *parent = 0);

    void start();
    void stop();

    bool tuneAvailable();
    bool sync(QList<Device *> deviceList);
    void executeAction(Device* device, const Action &action);

private:
    TuneManager *m_manager;
    int m_id;

    QHash<int, QVariantMap> m_requests;

    QByteArray formatResponse(int commandId, const QVariantMap &responseParams = QVariantMap());
    QByteArray formatErrorResponse(int commandId, const QString &errorMessage);
    void handleResponse(const QVariantMap &response);

signals:
    void connectionStatusChanged(const bool &connectionStatus);
    void gotTuneSync(const QVariantMap &params);
    void gotMoodSync(const QVariantMap &params);
    void gotActionResponse(const QVariantMap &response);

private slots:
    void processData(const QByteArray &data);
};

#endif // JSONRPCSERVER_H
